/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

#include "main.h"
#include "OutgoingContactRequest.h"
#include "ContactsManager.h"
#include "ContactUser.h"
#include "IncomingRequestManager.h"
#include "protocol/ContactRequestClient.h"
#include "tor/TorControlManager.h"

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

    return requestForUser(user);
}

OutgoingContactRequest::OutgoingContactRequest(ContactUser *u)
    : QObject(u), user(u), m_client(0)
{
    Q_ASSERT(user->isContactRequest());

    emit contactsManager->outgoingRequestAdded(this);

    attemptAutoAccept();

    if (status() < FirstResult)
    {
        connect(torManager, SIGNAL(socksReady()), SLOT(startConnection()));
        if (torManager->isSocksReady())
            startConnection();
    }
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

    IncomingContactRequest *incomingReq = contactsManager->incomingRequests->requestFromHostname(hostname);
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

    emit rejected(reason);
}

void OutgoingContactRequest::requestRejected(int reason)
{
    if (m_client->response() == ContactRequestClient::Rejected)
        reject();
    else
        reject(true, tr("An error occurred with the contact request (code: %1)").arg(reason, 0, 16));
}
