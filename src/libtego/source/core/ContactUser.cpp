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

#include "ed25519.hpp"
#include "context.hpp"
#include "user.hpp"
#include "globals.hpp"

ContactUser::ContactUser(UserIdentity *ident, const QString& hostname, Status status, QObject *parent)
    : QObject(parent)
    , identity(ident)
    , m_connection(0)
    , m_outgoingSocket(0)
    , m_status(status)
    , m_lastReceivedChatID(0)
    , m_contactRequest(0)
    , m_conversation(0)
    , m_hostname(hostname)
{
    Q_ASSERT(hostname.endsWith(".onion"));

    const auto serviceId = hostname.chopped(tego::static_strlen(".onion"));

    m_conversation = new ConversationModel(this);
    m_conversation->setContact(this);

    updateStatus();
    updateOutgoingSocket();
}

void ContactUser::createContactRequest(const QString& msg)
{
    m_contactRequest = new OutgoingContactRequest(this, msg);

    connect(m_contactRequest, &OutgoingContactRequest::statusChanged, this, &ContactUser::updateStatus);
    connect(m_contactRequest, &OutgoingContactRequest::removed, this, &ContactUser::requestRemoved);
    connect(m_contactRequest, &OutgoingContactRequest::accepted, this, &ContactUser::requestAccepted);
    updateStatus();
}

void ContactUser::updateStatus()
{
    logger::trace();
    Status newStatus;
    if (m_contactRequest) {
        if (m_contactRequest->status() == OutgoingContactRequest::Error ||
            m_contactRequest->status() == OutgoingContactRequest::Rejected)
        {
            newStatus = RequestRejected;
            logger::trace();
        } else {
            newStatus = RequestPending;
            logger::trace();
        }
    } else if (m_connection && m_connection->isConnected()) {
        newStatus = Online;
        logger::trace();
    } else if (m_status == RequestRejected) {
        newStatus = RequestRejected;
    } else {
        newStatus = Offline;
        logger::trace();
    }

    if (newStatus == m_status)
    {
        logger::trace();
        return;
    }

    {
        auto userId = this->toTegoUserId();
        switch(newStatus)
        {
            case ContactUser::Online:
                tego::g_globals.context->callback_registry_.emit_user_status_changed(userId.release(), tego_user_status_online);
                break;
            case ContactUser::Offline:
                tego::g_globals.context->callback_registry_.emit_user_status_changed(userId.release(), tego_user_status_offline);
                break;
            default:

                // noop, other statuses are handled elsewhere
                break;
        }

    }
    m_status = newStatus;
    emit statusChanged();

    updateOutgoingSocket();
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
        TEGO_BUG() << "Called updateOutgoingSocket with an existing socket in Ready. This should've been deleted.";
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

    if (m_contactRequest && m_connection->purpose() == Protocol::Connection::Purpose::OutboundRequest) {
        qDebug() << "Sending contact request for " << m_hostname;
        m_contactRequest->sendRequest(m_connection);
    }

    updateStatus();
    if (isConnected()) {
        emit connected();
        emit connectionChanged(m_connection);
    }

    if (m_status != Online && m_status != RequestPending) {
        TEGO_BUG() << "Contact has a connection while in status" << m_status << "which is not expected.";
        m_connection->close();
    }
}

void ContactUser::onDisconnected()
{
    qDebug() << "Contact" << m_hostname << "disconnected";

    if (m_connection) {
        if (m_connection->isConnected()) {
            TEGO_BUG() << "onDisconnected called, but connection is still connected";
            return;
        }

        m_connection.clear();
    } else {
        TEGO_BUG() << "onDisconnected called without a connection";
    }

    updateStatus();
    emit disconnected();
    emit connectionChanged(m_connection);
}

QString ContactUser::hostname() const
{
    return m_hostname;
}

quint16 ContactUser::port() const
{
    return 9878;
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

    m_hostname = hostname;

    updateOutgoingSocket();
}

void ContactUser::deleteContact()
{
    /* Anything that uses ContactUser is required to either respond to the contactDeleted signal
     * synchronously, or make use of QWeakPointer. */

    qDebug() << "Deleting contact" << m_hostname;

    if (m_contactRequest) {
        qDebug() << "Cancelling request associated with contact to be deleted";
        m_contactRequest->cancel();
    }
    Q_ASSERT(!m_contactRequest);

    emit contactDeleted(this);

    deleteLater();
}

void ContactUser::requestAccepted()
{
    if (!m_contactRequest) {
        TEGO_BUG() << "Request accepted but ContactUser doesn't know an active request";
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
        TEGO_BUG() << "Connection is already assigned to this ContactUser";
        return;
    }

    if (connection->purpose() == Protocol::Connection::Purpose::KnownContact) {
        TEGO_BUG() << "Connection is already assigned to a contact";
        connection->close();
        return;
    }

    bool isOutbound = connection->direction() == Protocol::Connection::ClientSide;

    if (!connection->isConnected()) {
        TEGO_BUG() << "Connection assigned to contact but isn't connected; discarding";
        connection->close();
        return;
    }

    if (!connection->hasAuthenticatedAs(Protocol::Connection::HiddenServiceAuth, hostname())) {
        TEGO_BUG() << "Connection assigned to contact without matching authentication";
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
                TEGO_BUG() << "Outgoing contact request not unset after implicit accept during connection";
        } else if (!m_contactRequest && !knownToPeer) {
            qDebug() << "Contact says we're unknown; marking as rejected";
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
        TEGO_BUG() << "After resolving connection races, ContactUser still has two connections";
        connection->close();
        return;
    }

    qDebug() << "Assigned" << (isOutbound ? "outbound" : "inbound") << "connection to contact" << m_hostname;

    if (m_contactRequest && isOutbound) {
        if (!connection->setPurpose(Protocol::Connection::Purpose::OutboundRequest)) {
            qWarning() << "BUG: Failed setting connection purpose for request";
            connection->close();
            return;
        }
    } else {
        if (m_contactRequest && !isOutbound) {
            qDebug() << "Implicitly accepting outgoing contact request for" << m_hostname << "due to incoming connection";
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
        TEGO_BUG() << "Failed queuing invocation of onConnected method";
}

void ContactUser::clearConnection()
{
    if (!m_connection)
        return;

    disconnect(m_connection.data(), 0, this, 0);
    m_connection->close();
    m_connection.clear();
}

std::unique_ptr<tego_user_id_t> ContactUser::toTegoUserId() const
{
    // convert our hostname to just the service id raw string
    auto serviceIdString = this->hostname().chopped(tego::static_strlen(".onion")).toUtf8();
    // ensure valid service id
    auto serviceId = std::make_unique<tego_v3_onion_service_id>(serviceIdString.data(), serviceIdString.size());
    // create user id object from service id
    auto userId = std::make_unique<tego_user_id>(*serviceId.get());

    return userId;
}