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

#include <QAbstractListModel>
#include <QDateTime>
#include "core/ContactUser.h"

#ifdef PROTOCOL_NEW
#include "protocol/ChatChannel.h"
#endif

class ConversationModel : public QAbstractListModel
{
    Q_OBJECT
    Q_ENUMS(MessageStatus)

    Q_PROPERTY(ContactUser* contact READ contact WRITE setContact NOTIFY contactChanged)
    Q_PROPERTY(int unreadCount READ unreadCount RESET resetUnreadCount NOTIFY unreadCountChanged)

public:
#ifdef PROTOCOL_NEW
    typedef Protocol::ChatChannel::MessageId MessageId;
#else
    typedef quint16 MessageId;
#endif

    enum {
        TimestampRole = Qt::UserRole,
        IsOutgoingRole,
        StatusRole,
        SectionRole
    };

    enum MessageStatus {
        Received,
        Sending,
        Delivered,
        Error
    };

    ConversationModel(QObject *parent = 0);

    ContactUser *contact() const { return m_contact; }
    void setContact(ContactUser *contact);

    int unreadCount() const { return m_unreadCount; }
    Q_INVOKABLE void resetUnreadCount();

    virtual QHash<int,QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

public slots:
    void sendMessage(const QString &text);
    void clear();

signals:
    void contactChanged();
    void unreadCountChanged();

private slots:
#ifdef PROTOCOL_NEW
    void messageReceived(const QString &text, const QDateTime &time, MessageId id);
    void messageDelivered(MessageId id);
#else
    void receiveMessage(const ChatMessageData &message);
    void messageReply();
#endif
    void onContactStatusChanged();

private:
    struct MessageData {
        QString text;
        QDateTime time;
        MessageId identifier;
        MessageStatus status;
    };

    ContactUser *m_contact;
    QList<MessageData> messages;
#ifndef PROTOCOL_NEW
    MessageId lastReceivedId;
#endif
    int m_unreadCount;

    int indexOfIdentifier(MessageId identifier, bool isOutgoing) const;
};

#endif

