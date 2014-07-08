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

#include "ContactUser.h"
#include "UserIdentity.h"
#include "ContactsManager.h"
#include "utils/SecureRNG.h"
#include "protocol/GetSecretCommand.h"
#include "protocol/ChatMessageCommand.h"
#include "protocol/OutgoingContactSocket.h"
#include "protocol/ProtocolConstants.h"
#include "core/ContactIDValidator.h"
#include "core/OutgoingContactRequest.h"
#include <QtDebug>
#include <QBuffer>
#include <QDateTime>

ContactUser::ContactUser(UserIdentity *ident, int id, QObject *parent)
    : QObject(parent)
    , identity(ident)
    , uniqueID(id)
    , m_lastReceivedChatID(0)
    , m_contactRequest(0)
    , m_outgoingSocket(0)
    , m_settings(0)
{
    Q_ASSERT(uniqueID >= 0);

    m_settings = new SettingsObject(QStringLiteral("contacts.%1").arg(uniqueID));
    connect(m_settings, &SettingsObject::modified, this, &ContactUser::onSettingsModified);

    m_conn = new ProtocolSocket(this);
    connect(m_conn, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_conn, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

    loadContactRequest();
    updateStatus();
}

void ContactUser::loadContactRequest()
{
    if (m_contactRequest)
        return;

    if (m_settings->read("request.status") != QJsonValue::Undefined) {
        m_contactRequest = new OutgoingContactRequest(this);
        connect(m_contactRequest, SIGNAL(statusChanged(int,int)), SLOT(updateStatus()));
        connect(m_contactRequest, SIGNAL(removed()), SLOT(requestRemoved()));
        updateStatus();
    }
}

ContactUser *ContactUser::addNewContact(UserIdentity *identity, int id)
{
    ContactUser *user = new ContactUser(identity, id);
    user->settings()->write("whenCreated", QDateTime::currentDateTime());

    /* Generate the local secret and set it */
    user->settings()->write("localSecret", Base64Encode(SecureRNG::random(16)));

    return user;
}

void ContactUser::updateStatus()
{
    Status newStatus;
    if (m_contactRequest) {
        if (m_contactRequest->status() == OutgoingContactRequest::Error ||
            m_contactRequest->status() == OutgoingContactRequest::Rejected)
        {
            newStatus = RequestRejected;
        } else {
            newStatus = RequestPending;
        }
    } else {
        newStatus = m_conn->isConnected() ? Online : Offline;
    }

    if (newStatus == m_status)
        return;

    m_status = newStatus;
    emit statusChanged();

    if (m_status == Offline)
        setupOutgoingSocket();
}

void ContactUser::onSettingsModified(const QString &key, const QJsonValue &value)
{
    Q_UNUSED(value);
    if (key == QLatin1String("nickname"))
        emit nicknameChanged();
}

void ContactUser::setupOutgoingSocket()
{
    if (m_status != Offline)
        return;

    QByteArray secret = m_settings->read<Base64Encode>("remoteSecret");
    if (secret.isEmpty() || hostname().isEmpty() || !port())
        return;

    // Refuse to make outgoing connections to the local hostname
    if (hostname() == identity->hostname())
        return;

    if (!m_outgoingSocket) {
        qDebug() << "Creating outgoing connection socket for contact";
        m_outgoingSocket = new OutgoingContactSocket(this);
        // TODO: Need proper UI and handling for authenticationFailed and versionNegotiationFailed
        connect(m_outgoingSocket, SIGNAL(socketReady(QTcpSocket*)), SLOT(incomingProtocolSocket(QTcpSocket*)));
    }

    m_outgoingSocket->setAuthentication(Protocol::PurposePrimary, secret);
    m_outgoingSocket->connectToHost(hostname(), port());
}

void ContactUser::onConnected()
{
    m_settings->write("lastConnected", QDateTime::currentDateTime());

    if (m_contactRequest)
    {
        qDebug() << "Implicitly accepting outgoing contact request for" << uniqueID << "from primary connection";

        m_contactRequest->accept();
        updateStatus();
        Q_ASSERT(status() != RequestPending);
    }

    if (m_settings->read("remoteSecret") == QJsonValue::Undefined)
    {
        qDebug() << "Requesting remote secret from user" << uniqueID;
        GetSecretCommand *command = new GetSecretCommand(this);
        command->send(conn());
    }

    updateStatus();
    emit connected();
}

void ContactUser::onDisconnected()
{
    qDebug() << "Contact" << uniqueID << "disconnected";
    m_settings->write("lastConnected", QDateTime::currentDateTime());

    updateStatus();
    emit disconnected();
}

SettingsObject *ContactUser::settings()
{
    return m_settings;
}

QString ContactUser::nickname() const
{
    return m_settings->read("nickname").toString();
}

void ContactUser::setNickname(const QString &nickname)
{
    m_settings->write("nickname", nickname);
}

QString ContactUser::hostname() const
{
    return m_settings->read("hostname").toString();
}

quint16 ContactUser::port() const
{
    return m_settings->read("port", 9878).toInt();
}

QString ContactUser::contactID() const
{
    return ContactIDValidator::idFromHostname(hostname());
}

void ContactUser::setHostname(const QString &hostname)
{
    QString fh = hostname;

    if (!hostname.endsWith(QLatin1String(".onion")))
        fh.append(QLatin1String(".onion"));

    m_settings->write("hostname", fh);
    setupOutgoingSocket();
}

void ContactUser::deleteContact()
{
    /* Anything that uses ContactUser is required to either respond to the contactDeleted signal
     * synchronously, or make use of QWeakPointer. */

    qDebug() << "Deleting contact" << uniqueID;

    if (m_contactRequest)
    {
        qDebug() << "Cancelling request associated with contact to be deleted";
        m_contactRequest->cancel();
        m_contactRequest->deleteLater();
    }

    emit contactDeleted(this);

    m_conn->disconnect();
    delete m_conn;
    m_conn = 0;

    m_settings->undefine();
    deleteLater();
}

void ContactUser::requestRemoved()
{
    if (m_contactRequest) {
        m_contactRequest->deleteLater();
        m_contactRequest = 0;
        updateStatus();
    }
}

static bool isOutgoing(QTcpSocket *socket)
{
    return socket->inherits("Tor::TorSocket");
}

void ContactUser::incomingProtocolSocket(QTcpSocket *socket)
{
    /* Per protocol.txt, to resolve connection races:
     *
     * - If the remote peer establishes a command connection prior to
     *   sending authentication data for an outgoing attempt, the outgoing
     *   attempt is aborted and the peer's connection is used.
     *
     * - If the existing connection is more than 30 seconds old, measured
     *   from the time authentication is sent, it is replaced and closed.
     *
     * - Otherwise, both peers close the connection for which the server's
     *   onion-formatted hostname is considered less by a strcmp function.
     */

    if (!isOutgoing(socket) && m_outgoingSocket && !m_outgoingSocket->isAuthenticationPending()) {
        // Abort connection attempt and use this socket
        m_outgoingSocket->disconnect();
        m_outgoingSocket->deleteLater();
        m_outgoingSocket = 0;
    }

    if (conn()->isConnected() && isOutgoing(conn()->socket()) != isOutgoing(socket) &&
        conn()->connectedDuration() < 30)
    {
        // Fall back to comparing onion hostnames to decide which connection to keep
        bool keepOutgoing = QString::compare(hostname(), identity->hostname()) < 0;
        if (isOutgoing(socket) != keepOutgoing) {
            qDebug() << "Discarding new protocol connection because existing one is too recent";
            socket->setParent(this);
            socket->abort();
            socket->deleteLater();
        } else {
            qDebug() << "Replacing existing (but recent) protocol connection";
            conn()->setSocket(socket);
        }
    } else {
        conn()->setSocket(socket);
    }

    if (isOutgoing(socket) && m_outgoingSocket) {
        m_outgoingSocket->deleteLater();
        m_outgoingSocket = 0;
    }
}

