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
#include "utils/Useful.h"
#include "core/ContactIDValidator.h"
#include "core/OutgoingContactRequest.h"
#include "core/ConversationModel.h"
#include "tor/HiddenService.h"
#include "protocol/OutboundConnector.h"
#include <QtDebug>
#include <QDateTime>
#include <QTcpSocket>
#include <QtEndian>

ContactUser::ContactUser(UserIdentity *ident, int id, QObject *parent)
    : QObject(parent)
    , identity(ident)
    , uniqueID(id)
    , m_connection(0)
    , m_outgoingSocket(0)
    , m_status(Offline)
    , m_lastReceivedChatID(0)
    , m_contactRequest(0)
    , m_settings(0)
    , m_conversation(0)
{
    Q_ASSERT(uniqueID >= 0);

    m_settings = new SettingsObject(QStringLiteral("contacts.%1").arg(uniqueID));
    connect(m_settings, &SettingsObject::modified, this, &ContactUser::onSettingsModified);

    m_conversation = new ConversationModel(this);
    m_conversation->setContact(this);

    loadContactRequest();
    updateStatus();
    updateOutgoingSocket();
}

ContactUser::~ContactUser()
{
    delete m_settings;
}

void ContactUser::loadContactRequest()
{
    if (m_contactRequest)
        return;

    if (m_settings->read("request.status") != QJsonValue::Undefined) {
        m_contactRequest = new OutgoingContactRequest(this);
        connect(m_contactRequest, &OutgoingContactRequest::statusChanged, this, &ContactUser::updateStatus);
        connect(m_contactRequest, &OutgoingContactRequest::removed, this, &ContactUser::requestRemoved);
        connect(m_contactRequest, &OutgoingContactRequest::accepted, this, &ContactUser::requestAccepted);
        updateStatus();
    }
}

ContactUser *ContactUser::addNewContact(UserIdentity *identity, int id)
{
    ContactUser *user = new ContactUser(identity, id);
    user->settings()->write("whenCreated", QDateTime::currentDateTime());

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
    } else if (m_connection && m_connection->isConnected()) {
        newStatus = Online;
    } else if (settings()->read("rejected").toBool()) {
        newStatus = RequestRejected;
    } else if (settings()->read("sentUpgradeNotification").toBool()) {
        newStatus = Outdated;
    } else {
        newStatus = Offline;
    }

    if (newStatus == m_status)
        return;

    m_status = newStatus;
    emit statusChanged();

    updateOutgoingSocket();
}

void ContactUser::onSettingsModified(const QString &key, const QJsonValue &value)
{
    Q_UNUSED(value);
    if (key == QLatin1String("nickname"))
        emit nicknameChanged();
}

void ContactUser::updateOutgoingSocket()
{
    if (m_status != Offline && m_status != RequestPending) {
        if (m_outgoingSocket) {
            m_outgoingSocket->disconnect(this);
            m_outgoingSocket->abort();
            m_outgoingSocket->deleteLater();
            m_outgoingSocket = 0;
        }
        return;
    }

    // Refuse to make outgoing connections to the local hostname
    if (hostname() == identity->hostname())
        return;

    if (m_outgoingSocket && m_outgoingSocket->status() == Protocol::OutboundConnector::Ready) {
        BUG() << "Called updateOutgoingSocket with an existing socket in Ready. This should've been deleted.";
        m_outgoingSocket->disconnect(this);
        m_outgoingSocket->deleteLater();
        m_outgoingSocket = 0;
    }

    if (!m_outgoingSocket) {
        m_outgoingSocket = new Protocol::OutboundConnector(this);
        m_outgoingSocket->setAuthPrivateKey(identity->hiddenService()->privateKey());
        connect(m_outgoingSocket, &Protocol::OutboundConnector::ready, this,
            [this]() {
                assignConnection(m_outgoingSocket->takeConnection());
            }
        );

        /* As an ugly hack, because Ricochet 1.0.x versions have no way to notify about
         * protocol issues, and it's not feasible to support both protocols for this
         * tiny upgrade period:
         *
         * The first time we make an outgoing connection to an existing contact, if they
         * are using the old version, send a chat message that lets them know about the
         * new version, then disconnect. This message is only sent once per contact.
         *
         * XXX: This logic should be removed an appropriate amount of time after the new
         * protocol has been released.
         */
        connect(m_outgoingSocket, &Protocol::OutboundConnector::oldVersionNegotiated, this,
            [this](QTcpSocket *socket) {
                if (m_settings->read("sentUpgradeNotification").toBool())
                    return;
                QByteArray secret = m_settings->read<Base64Encode>("remoteSecret");
                if (secret.size() != 16)
                    return;

                static const char upgradeMessage[] =
                    "[automatic message] I'm using a newer version of Ricochet that is not "
                    "compatible with yours. This is a one-time change to help improve Ricochet. "
                    "See https://ricochet.im/upgrade for instructions on getting the latest "
                    "version. Once you have upgraded, I will be able to see your messages again.";
                uchar command[] = {
                    0x00, 0x00, 0x10, 0x00, 0x00, 0x01, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
                };

                qToBigEndian(quint16(sizeof(upgradeMessage) + 7), command);
                qToBigEndian(quint16(sizeof(upgradeMessage) - 1), command + sizeof(command) - sizeof(quint16));

                QByteArray data;
                data.append((char)0x00);
                data.append(secret);
                data.append(reinterpret_cast<const char*>(command), sizeof(command));
                data.append(upgradeMessage);
                socket->write(data);

                m_settings->write("sentUpgradeNotification", true);
                updateStatus();
            }
        );
    }

    m_outgoingSocket->connectToHost(hostname(), port());
}

void ContactUser::onConnected()
{
    if (!m_connection || !m_connection->isConnected()) {
        /* This case can happen if disconnected very quickly after connecting,
         * before the (queued) slot has been called. Ignore the signal.
         */
        return;
    }

    m_settings->write("lastConnected", QDateTime::currentDateTime());

    if (m_contactRequest && m_connection->purpose() == Protocol::Connection::Purpose::OutboundRequest) {
        qDebug() << "Sending contact request for" << uniqueID << nickname();
        m_contactRequest->sendRequest(m_connection);
    }

    if (!m_settings->read("sentUpgradeNotification").isNull())
        m_settings->unset("sentUpgradeNotification");

    /* The 'rejected' mark comes from failed authentication to someone who we thought was a known
     * contact. Normally, it would mean that you were removed from that person's contacts. It's
     * possible for this to be undone; for example, if that person sends you a new contact request,
     * it will be automatically accepted. If this happens, unset the 'rejected' flag for correct UI.
     */
    if (m_settings->read("rejected").toBool()) {
        qDebug() << "Contact had marked us as rejected, but now they've connected again. Re-enabling.";
        m_settings->unset("rejected");
    }

    updateStatus();
    if (isConnected()) {
        emit connected();
        emit connectionChanged(m_connection);
    }

    if (m_status != Online && m_status != RequestPending) {
        BUG() << "Contact has a connection while in status" << m_status << "which is not expected.";
        m_connection->close();
    }
}

void ContactUser::onDisconnected()
{
    qDebug() << "Contact" << uniqueID << "disconnected";
    m_settings->write("lastConnected", QDateTime::currentDateTime());

    if (m_connection) {
        if (m_connection->isConnected()) {
            BUG() << "onDisconnected called, but connection is still connected";
            return;
        }

        m_connection.clear();
    } else {
        BUG() << "onDisconnected called without a connection";
    }

    updateStatus();
    emit disconnected();
    emit connectionChanged(m_connection);
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
    updateOutgoingSocket();
}

void ContactUser::deleteContact()
{
    /* Anything that uses ContactUser is required to either respond to the contactDeleted signal
     * synchronously, or make use of QWeakPointer. */

    qDebug() << "Deleting contact" << uniqueID;

    if (m_contactRequest) {
        qDebug() << "Cancelling request associated with contact to be deleted";
        m_contactRequest->cancel();
    }
    Q_ASSERT(!m_contactRequest);

    emit contactDeleted(this);

    m_settings->undefine();
    deleteLater();
}

void ContactUser::requestAccepted()
{
    if (!m_contactRequest) {
        BUG() << "Request accepted but ContactUser doesn't know an active request";
        return;
    }

    if (m_connection) {
        m_connection->setPurpose(Protocol::Connection::Purpose::KnownContact);
        emit connected();
    }

    requestRemoved();
}

void ContactUser::requestRemoved()
{
    if (m_contactRequest) {
        m_contactRequest->deleteLater();
        m_contactRequest = 0;
        updateStatus();
    }
}

void ContactUser::assignConnection(const QSharedPointer<Protocol::Connection> &connection)
{
    if (connection == m_connection) {
        BUG() << "Connection is already assigned to this ContactUser";
        return;
    }

    if (connection->purpose() == Protocol::Connection::Purpose::KnownContact) {
        BUG() << "Connection is already assigned to a contact";
        connection->close();
        return;
    }

    bool isOutbound = connection->direction() == Protocol::Connection::ClientSide;

    if (!connection->isConnected()) {
        BUG() << "Connection assigned to contact but isn't connected; discarding";
        connection->close();
        return;
    }

    if (!connection->hasAuthenticatedAs(Protocol::Connection::HiddenServiceAuth, hostname())) {
        BUG() << "Connection assigned to contact without matching authentication";
        connection->close();
        return;
    }

    /* KnownToPeer is set for an outbound connection when the remote end indicates
     * that it knows us as a contact. If this is set, we can assume that the
     * connection is fully built and will be kept open.
     *
     * If this isn't a request and KnownToPeer is not set, the connection has
     * effectively failed: it will be timed out and closed without a purpose.
     * This probably means that peer removed us a contact.
     */
    if (isOutbound) {
        bool knownToPeer = connection->hasAuthenticated(Protocol::Connection::KnownToPeer);
        if (m_contactRequest && knownToPeer) {
            m_contactRequest->accept();
            if (m_contactRequest)
                BUG() << "Outgoing contact request not unset after implicit accept during connection";
        } else if (!m_contactRequest && !knownToPeer) {
            qDebug() << "Contact says we're unknown; marking as rejected";
            settings()->write("rejected", true);
            connection->close();
            updateStatus();
            updateOutgoingSocket();
            return;
        }
    }

    if (m_connection && !m_connection->isConnected()) {
        qDebug() << "Replacing dead connection with new connection";
        clearConnection();
    }

    /* To resolve a race if two contacts try to connect at the same time:
     *
     * If the existing connection is in the same direction as the new one,
     * always use the new one.
     */
    if (m_connection && connection->direction() == m_connection->direction()) {
        qDebug() << "Replacing existing connection with contact because the new one goes the same direction";
        clearConnection();
    }

    /* If the existing connection is more than 30 seconds old, measured from
     * when it was successfully established, it's replaced with the new one.
     */
    if (m_connection && m_connection->age() > 30) {
        qDebug() << "Replacing existing connection with contact because it's more than 30 seconds old";
        clearConnection();
    }

    /* Otherwise, close the connection for which the server's onion-formatted
     * hostname compares less with a strcmp function
     */
    bool preferOutbound = QString::compare(hostname(), identity->hostname()) < 0;
    if (m_connection) {
        if (isOutbound == preferOutbound) {
            // New connection wins
            clearConnection();
        } else {
            // Old connection wins
            qDebug() << "Closing new connection with contact because the old connection won comparison";
            connection->close();
            return;
        }
    }

     /* If this connection is inbound and we have an outgoing connection attempt,
      * use the inbound connection if we haven't sent authentication yet, or if
      * we would lose the strcmp comparison above.
      */
    if (!isOutbound && m_outgoingSocket) {
        if (m_outgoingSocket->status() != Protocol::OutboundConnector::Authenticating || !preferOutbound) {
            // Inbound connection wins; outbound connection attempt will abort when status changes
            qDebug() << "Aborting outbound connection attempt because we got an inbound connection instead";
        } else {
            // Outbound attempt wins
            qDebug() << "Closing inbound connection with contact because the pending outbound connection won comparison";
            connection->close();
            return;
        }
    }

    if (m_connection) {
        BUG() << "After resolving connection races, ContactUser still has two connections";
        connection->close();
        return;
    }

    qDebug() << "Assigned" << (isOutbound ? "outbound" : "inbound") << "connection to contact" << uniqueID;

    if (m_contactRequest && isOutbound) {
        if (!connection->setPurpose(Protocol::Connection::Purpose::OutboundRequest)) {
            qWarning() << "BUG: Failed setting connection purpose for request";
            connection->close();
            return;
        }
    } else {
        if (m_contactRequest && !isOutbound) {
            qDebug() << "Implicitly accepting outgoing contact request for" << uniqueID << "due to incoming connection";
            m_contactRequest->accept();
        }

        if (!connection->setPurpose(Protocol::Connection::Purpose::KnownContact)) {
            qWarning() << "BUG: Failed setting connection purpose";
            connection->close();
            return;
        }
    }

    m_connection = connection;

    /* Use a queued connection to onDisconnected, because it clears m_connection.
     * If we cleared that immediately, it would be possible for the value to change
     * effectively any time we call into protocol code, which would be dangerous.
     */
    connect(m_connection.data(), &Protocol::Connection::closed, this, &ContactUser::onDisconnected, Qt::QueuedConnection);

    /* Delay the call to onConnected to allow protocol code to finish before everything
     * kicks in. In particular, this is important to allow AuthHiddenServiceChannel to
     * respond before other channels are created. */
    if (!metaObject()->invokeMethod(this, "onConnected", Qt::QueuedConnection))
        BUG() << "Failed queuing invocation of onConnected method";
}

void ContactUser::clearConnection()
{
    if (!m_connection)
        return;

    disconnect(m_connection.data(), 0, this, 0);
    m_connection->close();
    m_connection.clear();
}

