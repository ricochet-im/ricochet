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

#include "signals.hpp"
#include "context.hpp"
#include "ed25519.hpp"
#include "globals.hpp"

using tego::g_globals;

#include "UserIdentity.h"
#include "tor/TorControl.h"
#include "tor/HiddenService.h"
#include "core/ContactIDValidator.h"
#include "core/ContactUser.h"
#include "protocol/Connection.h"
#include "utils/Useful.h"

using namespace Protocol;

UserIdentity::UserIdentity(int id, const QString& serviceID, QObject *parent)
    : QObject(parent)
    , uniqueID(id)
    , contacts(this)
    , m_hiddenService(0)
    , m_incomingServer(0)
{
    setupService(serviceID);
}

UserIdentity *UserIdentity::createIdentity(int uniqueID)
{
    // There is actually no support for multiple identities currently.
    Q_ASSERT(uniqueID == 0);
    if (uniqueID != 0)
        return 0;

    return new UserIdentity(uniqueID, "", {});
}

// TODO: Handle the error cases of this function in a useful way
void UserIdentity::setupService(const QString& serviceID)
{
    g_globals.context->set_host_user_state(tego_host_user_state_offline);

    QString keyData = serviceID;

    if (!keyData.isEmpty()) {
        CryptoKey key;
        if (!key.loadFromKeyBlob(keyData.toLatin1())) {
            qWarning() << "Cannot load service key from configuration";
            return;
        }
        m_hiddenService = new Tor::HiddenService(key, this);
    } else {
        m_hiddenService = new Tor::HiddenService(this);
        connect(m_hiddenService, &Tor::HiddenService::privateKeyChanged, this,
            [&]() {
                QString key = QString::fromLatin1(m_hiddenService->privateKey().encodedKeyBlob());
                const QByteArray rawKey = key.toUtf8();

                // convert keyblob string to tego_ed25519_private key
                std::unique_ptr<tego_ed25519_private_key_t> privateKey;
                tego_ed25519_private_key_from_ed25519_keyblob(
                    tego::out(privateKey),
                    rawKey.data(),
                    static_cast<size_t>(rawKey.size()),
                    tego::throw_on_error());

                g_globals.context->callback_registry_.emit_new_identity_created(privateKey.release());
            }
        );
    }

    g_globals.context->set_host_user_state(tego_host_user_state_connecting);

    Q_ASSERT(m_hiddenService);
    connect(m_hiddenService, SIGNAL(statusChanged(int,int)), SLOT(onStatusChanged(int,int)));

    // Generally, these are not used, and we bind to localhost and port 0
    // for an automatic (and portable) selection.
    QHostAddress address = QHostAddress::LocalHost;
    quint16 port = 0;

    m_incomingServer = new QTcpServer(this);
    if (!m_incomingServer->listen(address, port)) {
        // XXX error case
        qWarning() << "Failed to open incoming socket:" << m_incomingServer->errorString();
        return;
    }

    connect(m_incomingServer, &QTcpServer::newConnection, this, &UserIdentity::onIncomingConnection);

    m_hiddenService->addTarget(9878, m_incomingServer->serverAddress(), m_incomingServer->serverPort());
    g_globals.context->torControl->addHiddenService(m_hiddenService);
}

QString UserIdentity::hostname() const
{
    return m_hiddenService ? m_hiddenService->hostname() : QString();
}

QString UserIdentity::contactID() const
{
    return ContactIDValidator::idFromHostname(hostname());
}

void UserIdentity::onStatusChanged(int newStatus, int oldStatus)
{
    if (oldStatus == Tor::HiddenService::NotCreated && newStatus > oldStatus)
    {
        emit contactIDChanged();
    }
    emit statusChanged();
}

bool UserIdentity::isServiceOnline() const
{
    return m_hiddenService && m_hiddenService->status() == Tor::HiddenService::Online;
}

/* Handle an incoming connection to this service
 *
 * A Protocol::Connection is created to handle this socket. The
 * connection initially has a purpose of Unknown. It times out
 * and automatically closes after ConnectionPrivate::UnknownPurposeTimeout
 * seconds, unless the purpose is changed.
 *
 * If the connection successfully completes authentication,
 * handleIncomingAuthedConnection is called to link it to a ContactUser
 * (if applicable) and set the purpose.
 */
void UserIdentity::onIncomingConnection()
{
    while (m_incomingServer->hasPendingConnections()) {
        QTcpSocket *socket = m_incomingServer->nextPendingConnection();

        /* The localHostname property is used by Connection to determine the
         * server onion hostname that this socket is connected to, which is
         * used by the serverHostname() method.
         */
        socket->setProperty("localHostname", m_hiddenService->hostname());

        qDebug() << "Accepted new incoming connection";
        QSharedPointer<Connection> conn(new Connection(socket, Connection::ServerSide), &QObject::deleteLater);
        Q_ASSERT(socket->parent());

        m_incomingConnections.append(conn);
        Connection *connPtr = conn.data();

        /* When the connection is closed, if it's not claimed, take it out of the
         * incoming connection list and destroy the reference
         */
        connect(connPtr, &Connection::closed, this,
            [this,connPtr]() {
                QSharedPointer<Connection> inconn(takeIncomingConnection(connPtr));
                if (inconn)
                    qDebug() << "Deleting closed incoming connection that was never claimed by an owner";
            }
        );

        connect(connPtr, &Connection::authenticated, this,
            [this,connPtr](Connection::AuthenticationType type) {
                if (type == Connection::HiddenServiceAuth)
                    handleIncomingAuthedConnection(connPtr);
            }
        );

        emit incomingConnection(connPtr);
    }
}

void UserIdentity::handleIncomingAuthedConnection(Connection *conn)
{
    if (conn->purpose() != Connection::Purpose::Unknown)
        return;

    QString clientName = conn->authenticatedIdentity(Connection::HiddenServiceAuth);
    if (clientName.isEmpty()) {
        TEGO_BUG() << "Called to handle incoming authed connection without any authed name";
        return;
    }

    ContactUser *user = contacts.lookupHostname(clientName);
    if (!user) {
        // This client can start a contact request, for example. The purpose stays unknown, and the
        // connection will be killed if the purpose isn't changed before the timeout.
        qDebug() << "Have an incoming connection authenticated as unknown client" << clientName;
        return;
    }

    QSharedPointer<Connection> connPtr(takeIncomingConnection(conn));
    if (!connPtr) {
        TEGO_BUG() << "Called to handle incoming authed connection, but the connection is already out of the incoming list";
        return;
    }

    qDebug() << "Incoming connection authenticated as contact with hostname" << clientName;
    user->assignConnection(connPtr);
}

QSharedPointer<Connection> UserIdentity::takeIncomingConnection(Connection *match)
{
    for (auto it = m_incomingConnections.begin(); it != m_incomingConnections.end(); it++) {
        if (it->data() == match) {
            QSharedPointer<Connection> re = *it;
            m_incomingConnections.erase(it);
            return re;
        }
    }

    return QSharedPointer<Connection>();
}
