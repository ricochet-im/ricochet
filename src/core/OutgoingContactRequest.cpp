/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
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

#include "main.h"
#include "OutgoingContactRequest.h"
#include "ContactsManager.h"
#include "ContactUser.h"
#include "UserIdentity.h"
#include "IncomingRequestManager.h"
#include "protocol/ContactRequestClient.h"
#include "tor/TorControlManager.h"
#include <QDebug>

OutgoingContactRequest *OutgoingContactRequest::requestForUser(ContactUser *user)
{
    OutgoingContactRequest *request = reinterpret_cast<OutgoingContactRequest*>(user->property("contactRequest").value<void*>());
    if (!request)
    {
        if (!user->isContactRequest())
            return 0;

        request = new OutgoingContactRequest(user);
        user->setProperty("contactRequest", QVariant::fromValue(reinterpret_cast<void*>(request)));
    }

    return request;
}

OutgoingContactRequest *OutgoingContactRequest::createNewRequest(ContactUser *user, const QString &myNickname,
                                                                 const QString &message)
{
    Q_ASSERT(!user->isContactRequest());

    user->writeSetting(QLatin1String("request/status"), static_cast<int>(Pending));
    user->writeSetting(QLatin1String("request/myNickname"), myNickname);
    user->writeSetting(QLatin1String("request/message"), message);

    user->updateStatus();
    return requestForUser(user);
}

OutgoingContactRequest::OutgoingContactRequest(ContactUser *u)
    : QObject(u), user(u), m_client(0)
{
    Q_ASSERT(user->isContactRequest());

    emit user->identity->contacts.outgoingRequestAdded(this);

    attemptAutoAccept();

    if (status() < FirstResult)
    {
        connect(torManager, SIGNAL(socksReady()), SLOT(startConnection()));
        if (torManager->isSocksReady())
            startConnection();
    }
}

OutgoingContactRequest::~OutgoingContactRequest()
{
    Q_ASSERT(!m_client);
    user->setProperty("contactRequest", QVariant());
}

QString OutgoingContactRequest::myNickname() const
{
    return user->readSetting("request/myNickname").toString();
}

QString OutgoingContactRequest::message() const
{
    return user->readSetting("request/message").toString();
}

OutgoingContactRequest::Status OutgoingContactRequest::status() const
{
    return static_cast<Status>(user->readSetting("request/status").toInt());
}

QString OutgoingContactRequest::rejectMessage() const
{
    return user->readSetting("request/rejectMessage").toString();
}

void OutgoingContactRequest::setStatus(Status newStatus)
{
    Status oldStatus = status();
    if (newStatus == oldStatus)
        return;

    user->writeSetting("request/status", static_cast<int>(newStatus));
    emit statusChanged(newStatus, oldStatus);
}

void OutgoingContactRequest::attemptAutoAccept()
{
    /* Check if there is an existing incoming request that matches this one; if so, treat this as accepted
     * automatically and accept that incoming request for this user */
    QByteArray hostname = user->hostname().left(16).toLatin1();

    IncomingContactRequest *incomingReq = user->identity->contacts.incomingRequests.requestFromHostname(hostname);
    if (incomingReq)
    {
        qDebug() << "Automatically accepting an incoming contact request matching a newly created outgoing request";

        accept();
        incomingReq->accept(user);
    }
}

void OutgoingContactRequest::startConnection()
{
    if (m_client)
        return;

    qDebug() << "Starting outgoing contact request for" << user->uniqueID;

    m_client = new ContactRequestClient(user);

    connect(m_client, SIGNAL(accepted()), SLOT(accept()));
    connect(m_client, SIGNAL(rejected(int)), SLOT(requestRejected(int)));
    connect(m_client, SIGNAL(acknowledged()), SLOT(requestAcknowledged()));

    m_client->setMyNickname(myNickname());
    m_client->setMessage(message());
    m_client->sendRequest();
}

void OutgoingContactRequest::removeRequest()
{
    if (m_client)
    {
        m_client->disconnect(this);

        /* Any higher response indicates that the connection will be handled elsewhere */
        if (m_client->response() <= ContactRequestClient::Acknowledged)
            m_client->close();

        m_client->deleteLater();
        m_client = 0;
    }

    /* Clear the request settings */
    user->removeSetting("request");
}

void OutgoingContactRequest::accept()
{
    setStatus(Accepted);
    removeRequest();
    emit accepted();
}

void OutgoingContactRequest::reject(bool error, const QString &reason)
{
    user->writeSetting("request/rejectMessage", reason);
    setStatus(error ? Error : Rejected);

    if (m_client)
    {
        m_client->disconnect(this);
        m_client->close();
        m_client->deleteLater();
        m_client = 0;
    }

    emit rejected(reason);
}

void OutgoingContactRequest::cancel()
{
    removeRequest();
}

void OutgoingContactRequest::requestAcknowledged()
{
    setStatus(Acknowledged);
}

void OutgoingContactRequest::requestRejected(int reason)
{
    if (m_client->response() == ContactRequestClient::Rejected)
        reject();
    else
        reject(true, tr("An error occurred with the contact request (code: %1)").arg(reason, 0, 16));
}
