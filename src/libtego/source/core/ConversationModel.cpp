/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "error.hpp"
#include "file_hash.hpp"
#include "globals.hpp"
using tego::g_globals;

#include "ConversationModel.h"
#include "protocol/Connection.h"
#include "protocol/ChatChannel.h"
#include "protocol/FileChannel.h"
#include "utils/SecureRNG.h"
#include "utils/Useful.h"

ConversationModel::ConversationModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_contact(0)
    , messages({})
    , m_unreadCount(0)
    , lastMessageId(SecureRNG::randomInt(UINT32_MAX))

{
}

void ConversationModel::setContact(ContactUser *contact)
{
    if (contact == m_contact)
        return;

    beginResetModel();
    messages.clear();

    if (m_contact)
        disconnect(m_contact, 0, this, 0);
    m_contact = contact;
    if (m_contact) {
        auto connectChannel = [this](Protocol::Channel *channel) {
            if (channel->direction() == Protocol::Channel::Outbound)
            {
                connect(channel, &Protocol::Channel::invalidated, this, &ConversationModel::outboundChannelClosed);
                sendQueuedMessages();
            }

            if (Protocol::ChatChannel *chat = qobject_cast<Protocol::ChatChannel*>(channel))
            {
                connect(chat, &Protocol::ChatChannel::messageReceived, this, &ConversationModel::messageReceived);
                connect(chat, &Protocol::ChatChannel::messageAcknowledged, this, &ConversationModel::messageAcknowledged);
            }
            else if (auto fc = qobject_cast<Protocol::FileChannel*>(channel); fc != nullptr)
            {
                connect(fc, &Protocol::FileChannel::fileTransferRequestReceived, this, &ConversationModel::onFileTransferRequestReceived);
                connect(fc, &Protocol::FileChannel::fileTransferAcknowledged, this, &ConversationModel::onFileTransferAcknowledged);
                connect(fc, &Protocol::FileChannel::fileTransferRequestResponded, this, &ConversationModel::onFileTransferRequestResponded);
                connect(fc, &Protocol::FileChannel::fileTransferProgress, this, &ConversationModel::onFileTransferProgress);
                connect(fc, &Protocol::FileChannel::fileTransferFinished, this, &ConversationModel::onFileTransferFinished);
            }
        };

        auto connectConnection = [this,connectChannel]() {
            if (m_contact->connection()) {
                connect(m_contact->connection().data(), &Protocol::Connection::channelOpened, this, connectChannel);
                foreach (auto channel, m_contact->connection()->findChannels<Protocol::Channel>())
                    connectChannel(channel);
                sendQueuedMessages();
            }
        };

        connect(m_contact, &ContactUser::connected, this, connectConnection);
        connectConnection();
        connect(m_contact, &ContactUser::statusChanged,
                this, &ConversationModel::onContactStatusChanged);
    }

    endResetModel();
    emit contactChanged();
}

/* Get a channel of type T for a contact, if it doesn't exist create one
 * on error returns NULL */
template<typename T> T *findOrCreateChannelForContact(ContactUser *contact, Protocol::Channel::Direction direction) {
    T *channel = contact->connection()->findChannel<T>(direction);
    if (!channel) {
        /* create a new channel */
        channel = new T(direction, contact->connection().data());
        if (!channel->openChannel())
        {
            delete channel;
            channel = nullptr;
        }
    }
    return channel;
}


std::tuple<tego_file_transfer_id_t, std::unique_ptr<tego_file_hash_t>, tego_file_size_t> ConversationModel::sendFile(const QString &file_uri)
{
    logger::println("Sending file: {}", file_uri);

    MessageData message(File, file_uri, QDateTime::currentDateTime(), lastMessageId++, Queued);
    message.type = ConversationModel::MessageType::File;

    std::unique_ptr<tego_file_hash_t> fileHash;

    // calculate our file hash
    if(std::ifstream file(file_uri.toStdString(), std::ios::in | std::ios::binary); file.is_open())
    {
        fileHash = std::make_unique<tego_file_hash_t>(file);
        // copy for our message
        message.fileHash = *fileHash;
    }
    else
    {
        TEGO_THROW_MSG("Could not open file {}", file_uri);
    }

	// calculate file size
    const tego_file_size_t fileSize = static_cast<tego_file_size_t>(QFileInfo(file_uri).size());

    if (m_contact->connection())
    {
        logger::trace();
        auto channel = findOrCreateChannelForContact<Protocol::FileChannel>(m_contact, Protocol::Channel::Outbound);
        if (channel && channel->isOpened())
        {
            logger::trace();
            if (channel->sendFileWithId(message.text, message.fileHash, QDateTime(), message.identifier))
            {
                logger::trace();
                message.status = Sending;
            }
            else
            {
                logger::trace();
                message.status = Error;
            }
            message.attemptCount++;
        }
    }
    else
    {
        logger::trace();
    }

    beginInsertRows(QModelIndex(), 0, 0);
    messages.prepend(message);
    endInsertRows();
    prune();

    return {message.identifier, std::move(fileHash), fileSize};
}

tego_message_id_t ConversationModel::sendMessage(const QString &text)
{
    if (text.isEmpty())
        return 0;

    MessageData message(Message, text, QDateTime::currentDateTime(), lastMessageId++, Queued);

    if (m_contact->connection())
    {
        auto channel = findOrCreateChannelForContact<Protocol::ChatChannel>(m_contact, Protocol::Channel::Outbound);
        if (channel && channel->isOpened())
        {
            if (channel->sendChatMessageWithId(text, QDateTime(), message.identifier))
            {
                message.status = Sending;
            }
            else
            {
                message.status = Error;
            }
            message.attemptCount++;
        }
    }

    beginInsertRows(QModelIndex(), 0, 0);
    messages.prepend(message);
    endInsertRows();
    prune();

    return message.identifier;
}

void ConversationModel::acceptFile(tego_file_transfer_id_t id, const std::string& dest)
{
    TEGO_THROW_IF_FALSE(m_contact->connection());
    auto channel = findOrCreateChannelForContact<Protocol::FileChannel>(m_contact, Protocol::Channel::Inbound);
    TEGO_THROW_IF_NULL(channel);
    TEGO_THROW_IF_FALSE(channel->isOpened());

    channel->acceptFile(id, dest);
}

void ConversationModel::rejectFile(tego_file_transfer_id_t id)
{
    TEGO_THROW_IF_FALSE(m_contact->connection());
    auto channel = findOrCreateChannelForContact<Protocol::FileChannel>(m_contact, Protocol::Channel::Inbound);
    TEGO_THROW_IF_NULL(channel);
    TEGO_THROW_IF_FALSE(channel->isOpened());

    channel->rejectFile(id);
}

void ConversationModel::cancelTransfer(tego_file_transfer_id_t id)
{
    if(m_contact->connection())
    {
        // first try cancelling an inbound transfer
        if (auto channel = findOrCreateChannelForContact<Protocol::FileChannel>(m_contact, Protocol::Channel::Inbound);
            channel != nullptr)
        {
            if (channel->isOpened() && channel->cancelTransfer(id))
            {
                return;
            }
        }

        // next try cancelling an outbound transfer
        if (auto channel = findOrCreateChannelForContact<Protocol::FileChannel>(m_contact, Protocol::Channel::Outbound);
            channel != nullptr)
        {
            if (channel->isOpened() && channel->cancelTransfer(id))
            {
                return;
            }
        }
    }
    else if(auto it = std::find_if(messages.begin(), messages.end(), [=](auto& msg) {return msg.identifier == id;});
            it != messages.end())
    {
        messages.erase(it);
    }
    else
    {
        TEGO_THROW_MSG("Tego transfer {} does not exist", id);
    }
}


void ConversationModel::sendQueuedMessages()
{
    if (!m_contact->connection())
        return;

    auto chat_channel = findOrCreateChannelForContact<Protocol::ChatChannel>(m_contact, Protocol::Channel::Outbound);
    auto file_channel = findOrCreateChannelForContact<Protocol::FileChannel>(m_contact, Protocol::Channel::Outbound);

    // sendQueuedMessages is called at channelOpened

    // Iterate backwards, from oldest to newest messages
    for (int i = messages.size() - 1; i >= 0; i--)
    {
        auto& m = messages[i];
        if (m.status == Queued) {
            qDebug() << "Sending queued chat message";
            bool attempted = false;
            switch (m.type)
            {
                case ConversationModel::MessageType::Message:
                    if (chat_channel->isOpened())
                    {
                        m.status = chat_channel->sendChatMessageWithId(m.text, m.time, m.identifier) ? Sending : Error;
                        attempted = true;
                    }
                    break;
                case ConversationModel::MessageType::File:
                    if (file_channel->isOpened())
                    {
                        logger::println("Attempted to send queued file: {}", m.text);
                        m.status = file_channel->sendFileWithId(m.text, m.fileHash, m.time, m.identifier) ? Sending : Error;
                        attempted = true;
                    }
                    break;
                default:
                    TEGO_BUG() << "Rejected invalid message type";
                    break;
            };

            if (attempted)
            {
                m.attemptCount++;
                emit dataChanged(index(i, 0), index(i, 0));
            }
        }
    }
}

void ConversationModel::messageReceived(const QString &text, const QDateTime &time, MessageId id)
{
    // In rare cases an outgoing acknowledgement packet can be lost which
    // causes the other party to resend the message. Discard the duplicate.
    // We don't need to resend the old acknowledgement packet because
    // it is identical to the one for the duplicate message.
    for (int i = 0; i < messages.size() && i < 5; i++) {
        if (messages[i].status == Delivered) {
            break;
        }
        if (messages[i].identifier == id && messages[i].text == text) {
            qDebug() << "duplicate incoming message" << id;
            return;
        }
    }

    // To preserve conversation flow despite potentially high latency, incoming messages
    // are positioned above the last unacknowledged messages to the peer. We assume that
    // the peer hadn't seen any unacknowledged message when this message was sent.
    int row = 0;
    for (int i = 0; i < messages.size() && i < 5; i++) {
        if (messages[i].status != Sending && messages[i].status != Queued) {
            row = i;
            break;
        }
    }

    beginInsertRows(QModelIndex(), row, row);
    MessageData message(Message, text, time, id, Received);
    messages.insert(row, message);
    endInsertRows();
    prune();

    m_unreadCount++;
    emit unreadCountChanged();

    {
        // convert QString to raw utf8
        auto utf8Text = text.toUtf8();
        auto rawText = std::make_unique<char[]>(static_cast<unsigned int>(utf8Text.size()) + 1u);
        std::copy(utf8Text.begin(), utf8Text.end(), rawText.get());

        auto userId = this->m_contact->toTegoUserId();

        logger::println("Received Message : {}", rawText.get());

        g_globals.context->callback_registry_.emit_message_received(userId.release(), static_cast<tego_time_t>(time.toMSecsSinceEpoch()), id, rawText.release(), static_cast<size_t>(utf8Text.size()));
    }
}

void ConversationModel::messageAcknowledged(MessageId id, bool accepted)
{
    int row = indexOfIdentifier(id, true);
    if (row < 0)
        return;

    MessageData &data = messages[row];
    data.status = accepted ? Delivered : Error;
    emit dataChanged(index(row, 0), index(row, 0));

    auto userId = this->contact()->toTegoUserId();
    g_globals.context->callback_registry_.emit_message_acknowledged(userId.release(), id, (accepted ? TEGO_TRUE : TEGO_FALSE));
}

void ConversationModel::outboundChannelClosed()
{
    // Any messages that are Sending are moved back to Queued, so they
    // will be re-sent when we reconnect.
    for (int i = 0; i < messages.size(); i++) {
        if (messages[i].status != Sending)
            continue;
        if (messages[i].attemptCount >= 2) {
            qDebug() << "Outbound chat channel closed, and unacknowledged message has been tried twice already. Marking as error.";
            messages[i].status = Error;
        } else {
            qDebug() << "Outbound chat channel closed, putting unacknowledged chat message back in queue";
            messages[i].status = Queued;
        }
        emit dataChanged(index(i, 0), index(i, 0));
    }

    // Try to reopen the channel if we're still connected
    if (m_contact && m_contact->connection() && m_contact->connection()->isConnected()) {
        metaObject()->invokeMethod(this, "sendQueuedMessages", Qt::QueuedConnection);
    }
}

void ConversationModel::clear()
{
    if (messages.isEmpty())
        return;

    beginRemoveRows(QModelIndex(), 0, messages.size()-1);
    messages.clear();
    endRemoveRows();

    resetUnreadCount();
}

void ConversationModel::resetUnreadCount()
{
    if (m_unreadCount == 0)
        return;
    m_unreadCount = 0;
    emit unreadCountChanged();
}

void ConversationModel::onContactStatusChanged()
{
    // Update in case section has changed
    emit dataChanged(index(0, 0), index(rowCount()-1, 0), QVector<int>() << SectionRole);
}

void ConversationModel::onFileTransferRequestReceived(tego_file_transfer_id_t id, const QString& filename, tego_file_size_t fileSize, tego_file_hash_t hash)
{
    // user id
    auto userId = this->contact()->toTegoUserId();

    // filename
    auto utf8Filename = filename.toUtf8();
    const auto rawFilenameLength = static_cast<std::size_t>(utf8Filename.size());
    const auto rawFilenameSize = rawFilenameLength + 1; // for null terminator
    auto rawFilename = std::make_unique<char[]>(rawFilenameSize);
    std::copy(utf8Filename.begin(), utf8Filename.end(), rawFilename.get());
    rawFilename[rawFilenameLength] = 0;

    // filehash
    auto heapHash = std::make_unique<tego_file_hash_t>(hash);

    g_globals.context->callback_registry_.emit_file_transfer_request_received(
        userId.release(),
        id,
        rawFilename.release(),
        rawFilenameLength,
        fileSize,
        heapHash.release());
}

void ConversationModel::onFileTransferAcknowledged(tego_file_transfer_id_t id, bool accepted)
{
    int row = indexOfIdentifier(id, true);
    if (row < 0)
        return;

    MessageData &data = messages[row];
    data.status = accepted ? Delivered : Error;
    emit dataChanged(index(row, 0), index(row, 0));

    auto userId = this->contact()->toTegoUserId();
    g_globals.context->callback_registry_.emit_file_transfer_request_acknowledged(
        userId.release(),
        id,
        accepted ? TEGO_TRUE : TEGO_FALSE);
}

void ConversationModel::onFileTransferRequestResponded(tego_file_transfer_id_t id, tego_file_transfer_response_t response)
{
    auto userId = this->contact()->toTegoUserId();
    g_globals.context->callback_registry_.emit_file_transfer_request_response_received(
        userId.release(),
        id,
        response);
}

void ConversationModel::onFileTransferProgress(tego_file_transfer_id_t id, tego_file_transfer_direction_t direction, uint64_t bytesTransmitted, uint64_t bytesTotal)
{
    auto userId = this->contact()->toTegoUserId();
    g_globals.context->callback_registry_.emit_file_transfer_progress(
        userId.release(),
        id,
        direction,
        bytesTransmitted,
        bytesTotal);
}

void ConversationModel::onFileTransferFinished(tego_file_transfer_id_t id, tego_file_transfer_direction_t direction, tego_file_transfer_result_t result)
{
    auto userId = this->contact()->toTegoUserId();
    g_globals.context->callback_registry_.emit_file_transfer_complete(
        userId.release(),
        id,
        direction,
        result);
}

QHash<int,QByteArray> ConversationModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "text";
    roles[TimestampRole] = "timestamp";
    roles[IsOutgoingRole] = "isOutgoing";
    roles[StatusRole] = "status";
    roles[SectionRole] = "section";
    roles[TimespanRole] = "timespan";
    return roles;
}

int ConversationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return messages.size();
}

QVariant ConversationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= messages.size())
        return QVariant();

    const MessageData &message = messages[index.row()];

    switch (role) {
        case Qt::DisplayRole: return message.text;
        case TimestampRole: return message.time;
        case IsOutgoingRole: return message.status != Received;
        case StatusRole: return message.status;

        case SectionRole: {
            if (m_contact->status() == ContactUser::Online)
                return QString();
            if (index.row() < messages.size() - 1) {
                const MessageData &next = messages[index.row()+1];
                if (next.status != Received && next.status != Delivered)
                    return QString();
            }
            for (int i = 0; i <= index.row(); i++) {
                if (messages[i].status == Received || messages[i].status == Delivered)
                    return QString();
            }
            return QStringLiteral("offline");
        }
        case TimespanRole: {
            if (index.row() < messages.size() - 1)
                return messages[index.row() + 1].time.secsTo(messages[index.row()].time);
            else
                return -1;
        }
    }

    return QVariant();
}

int ConversationModel::indexOfIdentifier(MessageId identifier, bool isOutgoing) const
{
    for (int i = 0; i < messages.size(); i++) {
        if (messages[i].identifier == identifier && (messages[i].status != Received) == isOutgoing)
            return i;
    }
    return -1;
}

void ConversationModel::prune()
{
    const int history_limit = 1000;
    if (messages.size() > history_limit) {
        beginRemoveRows(QModelIndex(), history_limit, messages.size()-1);
        while (messages.size() > history_limit) {
            messages.removeLast();
        }
        endRemoveRows();
    }
}
