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

#include "ConversationModel.h"
#include <QDebug>
#ifdef PROTOCOL_NEW
# include "protocol/Connection.h"
# include "protocol/ChatChannel.h"
#else
# include "protocol/ChatMessageCommand.h"
#endif

ConversationModel::ConversationModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_contact(0)
#ifndef PROTOCOL_NEW
    , lastReceivedId(0)
#endif
    , m_unreadCount(0)
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
#ifdef PROTOCOL_NEW
        auto connectChannel = [this](Protocol::Channel *channel) {
            if (Protocol::ChatChannel *chat = qobject_cast<Protocol::ChatChannel*>(channel)) {
                connect(chat, &Protocol::ChatChannel::messageReceived, this, &ConversationModel::messageReceived);
                connect(chat, &Protocol::ChatChannel::messageAcknowledged, this, &ConversationModel::messageAcknowledged);

                if (chat->direction() == Protocol::Channel::Outbound)
                    sendQueuedMessages();
            }
        };

        auto connectConnection = [this,connectChannel]() {
            if (m_contact->connection()) {
                connect(m_contact->connection(), &Protocol::Connection::channelOpened, this, connectChannel);
                foreach (auto channel, m_contact->connection()->findChannels<Protocol::ChatChannel>())
                    connectChannel(channel);
                sendQueuedMessages();
            }
        };

        connect(m_contact, &ContactUser::connected, this, connectConnection);
        connectConnection();
#else
        connect(m_contact, SIGNAL(incomingChatMessage(ChatMessageData)), this,
                SLOT(receiveMessage(ChatMessageData)));
#endif
        connect(m_contact, &ContactUser::statusChanged,
                this, &ConversationModel::onContactStatusChanged);
    }

    endResetModel();
    emit contactChanged();
}

void ConversationModel::sendMessage(const QString &text)
{
    if (text.isEmpty())
        return;

#ifdef PROTOCOL_NEW
    MessageData message = { text, QDateTime::currentDateTime(), 0, Queued };

    if (m_contact->connection()) {
        auto channel = m_contact->connection()->findChannel<Protocol::ChatChannel>(Protocol::Channel::Outbound);
        if (!channel) {
            channel = new Protocol::ChatChannel(Protocol::Channel::Outbound, m_contact->connection());
            if (!channel->openChannel()) {
                message.status = Error;
                delete channel;
                channel = 0;
            }
        }

        if (channel && channel->isOpened()) {
            MessageId id = 0;
            if (channel->sendChatMessage(text, QDateTime(), id))
                message.status = Sending;
            else
                message.status = Error;
            message.identifier = id;
        }
    }

#else
    ChatMessageCommand *command = new ChatMessageCommand;
    connect(command, SIGNAL(commandFinished()), this, SLOT(messageReply()));
    command->send(m_contact->conn(), QDateTime::currentDateTime(), text, lastReceivedId);

    MessageData message = { text, QDateTime::currentDateTime(), command->identifier(), Sending };
#endif

    beginInsertRows(QModelIndex(), 0, 0);
    messages.prepend(message);
    endInsertRows();
}

#ifdef PROTOCOL_NEW
void ConversationModel::sendQueuedMessages()
{
    if (!m_contact->connection())
        return;

    // Quickly scan to see if we have any queued messages
    bool haveQueued = false;
    foreach (const MessageData &data, messages) {
        if (data.status == Queued) {
            haveQueued = true;
            break;
        }
    }

    if (!haveQueued)
        return;

    auto channel = m_contact->connection()->findChannel<Protocol::ChatChannel>(Protocol::Channel::Outbound);
    if (!channel) {
        channel = new Protocol::ChatChannel(Protocol::Channel::Outbound, m_contact->connection());
        if (!channel->openChannel()) {
            delete channel;
            return;
        }
    }

    // sendQueuedMessages is called at channelOpened
    if (!channel->isOpened())
        return;

    // Iterate backwards, from oldest to newest messages
    for (int i = messages.size() - 1; i >= 0; i--) {
        if (messages[i].status == Queued) {
            qDebug() << "Sending queued chat message";
            bool ok = false;
            if (messages[i].identifier)
                ok = channel->sendChatMessageWithId(messages[i].text, messages[i].time, messages[i].identifier);
            else
                ok = channel->sendChatMessage(messages[i].text, messages[i].time, messages[i].identifier);
            if (ok)
                messages[i].status = Sending;
            else
                messages[i].status = Error;
            emit dataChanged(index(i, 0), index(i, 0));
        }
    }
}

void ConversationModel::messageReceived(const QString &text, const QDateTime &time, MessageId id)
{
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
    MessageData message(text, time, id, Received);
    messages.insert(row, message);
    endInsertRows();

    m_unreadCount++;
    emit unreadCountChanged();
}

void ConversationModel::messageAcknowledged(MessageId id, bool accepted)
{
    int row = indexOfIdentifier(id, true);
    if (row < 0)
        return;

    MessageData &data = messages[row];
    data.status = accepted ? Delivered : Error;
    emit dataChanged(index(row, 0), index(row, 0));
}
#else
void ConversationModel::receiveMessage(const ChatMessageData &data)
{
    // If priorMessageID is non-zero, it represents the identifier of the last message
    // the peer had received when this message was sent. To help keep the flow of a
    // conversation despite latency, attempt to insert this message where the peer sees it.
    int row = 0;
    if (data.priorMessageID) {
        for (int i = 0; i < messages.size() && i < 5; i++) {
            if (messages[i].status == Received ||
                messages[i].identifier == data.priorMessageID)
            {
                row = i;
                break;
            }
        }
    }

    beginInsertRows(QModelIndex(), row, row);
    MessageData message = { data.text.trimmed(), data.when, data.messageID, Received };
    lastReceivedId = data.messageID;
    messages.insert(row, message);
    endInsertRows();

    m_unreadCount++;
    emit unreadCountChanged();
}

void ConversationModel::messageReply()
{
    ChatMessageCommand *command = qobject_cast<ChatMessageCommand*>(sender());
    if (!command)
        return;

    int row = indexOfIdentifier(command->identifier(), true);
    if (row < 0)
        return;

    MessageData &data = messages[row];
    data.status = Protocol::isSuccess(command->finalReplyState()) ? Delivered : Error;
    emit dataChanged(index(row, 0), index(row, 0));
}
#endif

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

QHash<int,QByteArray> ConversationModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "text";
    roles[TimestampRole] = "timestamp";
    roles[IsOutgoingRole] = "isOutgoing";
    roles[StatusRole] = "status";
    roles[SectionRole] = "section";
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

