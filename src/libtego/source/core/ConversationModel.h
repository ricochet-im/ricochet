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

#ifndef CONVERSATIONMODEL_H
#define CONVERSATIONMODEL_H

#include "core/ContactUser.h"
#include "protocol/ChatChannel.h"
#include "protocol/FileChannel.h"

class ConversationModel : public QAbstractListModel
{
    Q_OBJECT
public:
    typedef Protocol::ChatChannel::MessageId MessageId;
    static_assert(std::is_same_v<MessageId, tego_message_id_t>);

    enum {
        TimestampRole = Qt::UserRole,
        IsOutgoingRole,
        StatusRole,
        SectionRole,
        TimespanRole
    };

    enum MessageStatus {
        Received,
        Queued,
        Sending,
        Delivered,
        Error
    };

    enum MessageType {
        Message,
        File
    };

    ConversationModel(QObject *parent = 0);

    ContactUser *contact() const { return m_contact; }
    void setContact(ContactUser *contact);

    int unreadCount() const { return m_unreadCount; }
    void resetUnreadCount();

    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    std::tuple<tego_file_transfer_id_t, std::unique_ptr<tego_file_hash_t>, tego_file_size_t> sendFile(const QString &file_url);
    tego_message_id_t sendMessage(const QString &text);

    void acceptFile(tego_file_transfer_id_t id, const std::string& dest);
    void rejectFile(tego_file_transfer_id_t id);
    void cancelTransfer(tego_file_transfer_id_t id);

    void clear();

signals:
    void contactChanged();
    void unreadCountChanged();

private slots:
    void messageReceived(const QString &text, const QDateTime &time, MessageId id);
    void messageAcknowledged(MessageId id, bool accepted);
    void outboundChannelClosed();
    void sendQueuedMessages();
    void onContactStatusChanged();

    void onFileTransferRequestReceived(tego_file_transfer_id_t id, const QString& filename, tego_file_size_t fileSize, tego_file_hash_t hash);
    void onFileTransferAcknowledged(tego_file_transfer_id_t id, bool ack);
    void onFileTransferRequestResponded(tego_file_transfer_id_t id, tego_file_transfer_response_t response);
    void onFileTransferProgress(tego_file_transfer_id_t id, tego_file_transfer_direction_t direction, tego_file_size_t bytesTransmitted, tego_file_size_t bytesTotal);
    void onFileTransferFinished(tego_file_transfer_id_t id, tego_file_transfer_direction_t direction, tego_file_transfer_result_t result);

private:
    struct MessageData {
        MessageType type;
        QString text;
        tego_file_hash_t fileHash;
        QDateTime time;
        MessageId identifier;
        MessageStatus status;
        quint8 attemptCount;

        MessageData(MessageType type, const QString &text, const QDateTime &time, MessageId id, MessageStatus status)
            : type(type), text(text), time(time), identifier(id), status(status), attemptCount(0)
        {
        }
    };

    ContactUser *m_contact;
    QList<MessageData> messages;
    int m_unreadCount;

    // The peer might use recent message IDs between connections to handle
    // re-send. Start at a random ID to reduce chance of collisions, then increment
    MessageId lastMessageId;

    int indexOfIdentifier(MessageId identifier, bool isOutgoing) const;
    void prune();
};

#endif

