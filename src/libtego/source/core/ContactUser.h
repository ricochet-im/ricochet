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

#ifndef CONTACTUSER_H
#define CONTACTUSER_H

#include "protocol/Connection.h"

class UserIdentity;
class OutgoingContactRequest;
class ConversationModel;

namespace Protocol
{
    class OutboundConnector;
}

/* Represents a user on the contact list.
 * All persistent uses of a ContactUser instance must either connect to the
 * contactDeleted() signal, or use a QWeakPointer to track deletion. A ContactUser
 * can be removed at essentially any time. */

class ContactUser : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactUser)

    friend class ContactsManager;
    friend class OutgoingContactRequest;

public:
    enum Status
    {
        Online,
        Offline,
        RequestPending,
        RequestRejected,
        Outdated    // this doesn't seem to actually be used
    };

    UserIdentity * const identity;

    explicit ContactUser(UserIdentity *identity, const QString& hostname, Status status=Offline, QObject *parent = 0);

    const QSharedPointer<Protocol::Connection> &connection() { return m_connection; }
    bool isConnected() const { return status() == Online; }

    OutgoingContactRequest *contactRequest() { return m_contactRequest; }
    ConversationModel *conversation() { return m_conversation; }

    UserIdentity *getIdentity() const { return identity; }

    /* Hostname is in the onion hostname format, i.e. it ends with .onion */
    QString hostname() const;
    quint16 port() const;
    /* Contact ID in the ricochet: format */
    QString contactID() const;

    Status status() const { return m_status; }

    void deleteContact();

    std::unique_ptr<tego_user_id_t> toTegoUserId() const;

public slots:
    /* Assign a connection to this user
     *
     * The connection must be connected, and the peer must be authenticated and
     * must match this user. ContactUser will assume ownership of the connection,
     * and it will be closed and deleted when it's no longer used.
     *
     * It is valid to pass an incoming or outgoing connection. If there is already
     * a connection, protocol-specific rules are applied and the new connection
     * may be closed to favor the older one.
     *
     * If the existing connection is replaced, that is equivalent to disconnecting
     * and reconnectng immediately - any ongoing operations will fail and need to
     * be retried at a higher level.
     */
    void assignConnection(const QSharedPointer<Protocol::Connection> &connection);

    void setHostname(const QString &hostname);

    void updateStatus();

signals:
    void statusChanged();
    void connected();
    void disconnected();
    void connectionChanged(const QWeakPointer<Protocol::Connection> &connection);

    void nicknameChanged();
    void contactDeleted(ContactUser *user);

private slots:
    void onConnected();
    void onDisconnected();
    void requestRemoved();
    void requestAccepted();

private:
    QSharedPointer<Protocol::Connection> m_connection;
    Protocol::OutboundConnector *m_outgoingSocket;

    Status m_status;
    quint16 m_lastReceivedChatID;
    OutgoingContactRequest *m_contactRequest;
    ConversationModel *m_conversation;
    mutable QString m_hostname;

    /* See ContactsManager::addContact */
    static ContactUser *addNewContact(UserIdentity *identity, const QString& contactHostname);

    void createContactRequest(const QString& msg);
    void updateOutgoingSocket();

    void clearConnection();
};

Q_DECLARE_METATYPE(ContactUser*)

#endif // CONTACTUSER_H
