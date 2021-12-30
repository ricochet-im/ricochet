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

#ifndef USERIDENTITY_H
#define USERIDENTITY_H

#include "ContactsManager.h"

namespace Tor
{
    class HiddenService;
}

namespace Protocol
{
    class Connection;
}

class QTcpServer;

/* UserIdentity represents the local identity offered by the user.
 *
 * In particular, it represents the published hidden service, and
 * theoretically holds the list of contacts.
 *
 * At present, implementation (and settings) assumes that there is
 * only one identity, but some code is confusingly written to allow
 * for several.
 */
class UserIdentity : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserIdentity)

    friend class IdentityManager;
public:
    const int uniqueID;
    ContactsManager contacts;

    explicit UserIdentity(int uniqueID, const QString& serviceID, QObject *parent = 0);

    /* Properties */
    int getUniqueID() const { return uniqueID; }
    /* Hostname is .onion format, like ContactUser */
    QString hostname() const;
    QString contactID() const;

    ContactsManager *getContacts() { return &contacts; }

    /* State */
    bool isServiceOnline() const;
    Tor::HiddenService *hiddenService() const { return m_hiddenService; }

    /* Take ownership of an inbound connection. Returns the shared pointer to
     * the connection, and releases the reference held by UserIdentity. */
    QSharedPointer<Protocol::Connection> takeIncomingConnection(Protocol::Connection *connection);

signals:
    void statusChanged();
    void contactIDChanged(); // only possible during creation
    void incomingConnection(Protocol::Connection *connection);

private slots:
    void onStatusChanged(int newStatus, int oldStatus);
    void onIncomingConnection();

private:
    Tor::HiddenService *m_hiddenService;
    QTcpServer *m_incomingServer;
    QVector<QSharedPointer<Protocol::Connection>> m_incomingConnections;

    static UserIdentity *createIdentity(int uniqueID);

    void handleIncomingAuthedConnection(Protocol::Connection *connection);
    void setupService(const QString& serviceID);
};

Q_DECLARE_METATYPE(UserIdentity*)

#endif // USERIDENTITY_H
