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

#include "OutgoingContactRequest.h"
#include "ContactsManager.h"
#include "ContactUser.h"
#include "UserIdentity.h"
#include "IncomingRequestManager.h"
#include "utils/Useful.h"
#include "protocol/ContactRequestChannel.h"
#include "utils/Settings.h"

#include "ed25519.hpp"
#include "context.hpp"
#include "user.hpp"

OutgoingContactRequest *OutgoingContactRequest::createNewRequest(ContactUser *user, const QString &myNickname,
                                                                 const QString &message)
{
    Q_ASSERT(!user->contactRequest());

    SettingsObject *settings = user->settings();
    settings->write("request.myNickname", myNickname);
    settings->write("request.message", message);

    user->createContactRequest();
    Q_ASSERT(user->contactRequest());
    user->contactRequest()->setStatus(Pending);
    return user->contactRequest();
}

OutgoingContactRequest::OutgoingContactRequest(ContactUser *u)
    : QObject(u), user(u)
    , m_settings(new SettingsObject(u->settings(), QStringLiteral("request"), this))
    , m_status(Pending)
{
    emit user->identity->contacts.outgoingRequestAdded(this);

    attemptAutoAccept();
}

OutgoingContactRequest::~OutgoingContactRequest()
{
    user->setProperty("contactRequest", QVariant());
}

QString OutgoingContactRequest::myNickname() const
{
    return m_settings->read("myNickname").toString();
}

QString OutgoingContactRequest::message() const
{
    return m_settings->read("message").toString();
}

OutgoingContactRequest::Status OutgoingContactRequest::status() const
{
    return m_status;
}

QString OutgoingContactRequest::rejectMessage() const
{
    return m_settings->read("rejectMessage").toString();
}

void OutgoingContactRequest::setStatus(Status newStatus)
{
    Status oldStatus = status();
    if (newStatus == oldStatus)
        return;

    m_status = newStatus;
    emit statusChanged(newStatus, oldStatus);
}

void OutgoingContactRequest::attemptAutoAccept()
{
    /* Check if there is an existing incoming request that matches this one; if so, treat this as accepted
     * automatically and accept that incoming request for this user */
    QByteArray hostname = user->hostname().toLatin1();

    IncomingContactRequest *incomingReq = user->identity->contacts.incomingRequests.requestFromHostname(hostname);
    if (incomingReq)
    {
        qDebug() << "Automatically accepting an incoming contact request matching a newly created outgoing request";

        accept();
        incomingReq->accept(user);
    }
}

void OutgoingContactRequest::sendRequest(const QSharedPointer<Protocol::Connection> &connection)
{
    if (connection != user->connection()) {
        BUG() << "OutgoingContactRequest connection doesn't match the assigned user";
        return;
    }

    if (connection->purpose() != Protocol::Connection::Purpose::OutboundRequest) {
        BUG() << "OutgoingContactRequest told to use a connection of invalid purpose" << int(connection->purpose());
        return;
    }

    // XXX timeouts
    Protocol::ContactRequestChannel *channel = new Protocol::ContactRequestChannel(Protocol::Channel::Outbound, connection.data());
    connect(channel, &Protocol::ContactRequestChannel::requestStatusChanged,
            this, &OutgoingContactRequest::requestStatusChanged);

    // On any final response, the channel will be closed. Unless the purpose has been
    // changed (to KnownContact, on accept), close the connection at that time. That
    // will eventually trigger a retry via ContactUser if the request is still valid.
    connect(channel, &Protocol::Channel::invalidated, this,
        [this,connection]() {
            if (connection->isConnected() &&
                connection->purpose() == Protocol::Connection::Purpose::OutboundRequest)
            {
                qDebug() << "Closing connection attached to an OutgoingContactRequest because ContactRequestChannel was closed";
                connection->close();
            }
        }
    );

    if (!message().isEmpty())
        channel->setMessage(message());
    if (!myNickname().isEmpty())
        channel->setNickname(myNickname());

    if (!channel->openChannel()) {
        BUG() << "Channel for outgoing contact request failed";
        return;
    }
}

void OutgoingContactRequest::removeRequest()
{
    if (user->connection()) {
        Protocol::Channel *channel = user->connection()->findChannel<Protocol::ContactRequestChannel>();
        if (channel)
            channel->closeChannel();
    }

    /* Clear the request settings */
    m_settings->undefine();
    emit removed();
}

void OutgoingContactRequest::accept()
{
    setStatus(Accepted);
    emit accepted();
    removeRequest();
}

void OutgoingContactRequest::reject(bool error, const QString &reason)
{
    logger::println("reject -> error : {}, reason '{}'", error, reason);
    m_settings->write("rejectMessage", reason);
    setStatus(error ? Error : Rejected);

    if (user->connection()) {
        Protocol::Channel *channel = user->connection()->findChannel<Protocol::ContactRequestChannel>();
        if (channel)
            channel->closeChannel();
    }

    emit rejected(reason);
}

void OutgoingContactRequest::cancel()
{
    removeRequest();
}

void OutgoingContactRequest::requestStatusChanged(int status)
{
    using namespace Protocol::Data::ContactRequest;
    switch (status) {
        case Response::Pending:
            setStatus(Acknowledged);
            break;
        case Response::Accepted:
            accept();
            break;
        case Response::Rejected:
            reject();
            break;
        case Response::Error:
            reject(true);
            break;
        default:
            BUG() << "Unknown ContactRequest response status";
            break;
    }
}
