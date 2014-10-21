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

#include "IdentityManager.h"
#include "IncomingRequestManager.h"
#include "ContactsManager.h"
#include "OutgoingContactRequest.h"
#include "ContactIDValidator.h"
#include "protocol/ContactRequestServer.h"
#include <QDebug>

IncomingRequestManager::IncomingRequestManager(ContactsManager *c)
    : QObject(c), contacts(c)
{
    connect(this, SIGNAL(requestAdded(IncomingContactRequest*)), this, SIGNAL(requestsChanged()));
    connect(this, SIGNAL(requestRemoved(IncomingContactRequest*)), this, SIGNAL(requestsChanged()));
}

void IncomingRequestManager::loadRequests()
{
    SettingsObject settings(QStringLiteral("contactRequests"));

    foreach (const QString &host, settings.data().keys())
    {
        IncomingContactRequest *request = new IncomingContactRequest(this, host.toLatin1());
        request->load();

        m_requests.append(request);
        emit requestAdded(request);
    }
}

QList<QObject*> IncomingRequestManager::requestObjects() const
{
    QList<QObject*> re;
    re.reserve(m_requests.size());
    foreach (IncomingContactRequest *o, m_requests)
        re.append(o);
    return re;
}

IncomingContactRequest *IncomingRequestManager::requestFromHostname(const QByteArray &hostname)
{
    Q_ASSERT(!hostname.endsWith(".onion"));
    Q_ASSERT(hostname == hostname.toLower());

    for (QList<IncomingContactRequest*>::ConstIterator it = m_requests.begin(); it != m_requests.end(); ++it)
        if ((*it)->hostname() == hostname)
            return *it;

    return 0;
}

void IncomingRequestManager::addRequest(const QByteArray &hostname, const QByteArray &connSecret, ContactRequestServer *connection,
                                        const QString &nickname, const QString &message)
{
    if (isHostnameRejected(hostname))
    {
        qDebug() << "Rejecting contact request due to a blacklist match for" << hostname;

        if (connection)
            connection->sendRejection();

        return;
    }

    if (identityManager->lookupHostname(QString::fromLatin1(hostname)))
    {
        qDebug() << "Rejecting contact request from a local identity (?)";
        if (connection)
            connection->sendRejection();
        return;
    }

    IncomingContactRequest *request = requestFromHostname(hostname);
    bool newRequest = false;

    if (request)
    {
        /* Update the existing request */
        request->setConnection(connection);
        request->setRemoteSecret(connSecret);
        request->setNickname(nickname);
        request->setMessage(message);
        request->renew();
    }
    else
    {
        /* Create a new request */
        newRequest = true;

        request = new IncomingContactRequest(this, hostname, connection);
        request->setRemoteSecret(connSecret);
        request->setNickname(nickname);
        request->setMessage(message);
    }

    /* Check if this request matches any existing users, including any outgoing requests. */
    ContactUser *existingUser = contacts->lookupHostname(QString::fromLatin1(hostname));
    if (existingUser)
    {
        /* If the existing user is an outgoing contact request, that is considered accepted */
        if (existingUser->contactRequest())
            existingUser->contactRequest()->accept();

        /* This request is automatically accepted */
        request->accept(existingUser);
        return;
    }

    request->save();
    if (newRequest)
    {
        m_requests.append(request);
        emit requestAdded(request);
    }
}

void IncomingRequestManager::removeRequest(IncomingContactRequest *request)
{
    if (m_requests.removeOne(request))
        emit requestRemoved(request);

    request->deleteLater();
}

void IncomingRequestManager::addRejectedHost(const QByteArray &hostname)
{
    SettingsObject *settings = contacts->identity->settings();
    QJsonArray blacklist = settings->read<QJsonArray>("hostnameBlacklist");
    if (!blacklist.contains(QString::fromLatin1(hostname))) {
        blacklist.append(QString::fromLatin1(hostname));
        settings->write("hostnameBlacklist", blacklist);
    }
}

bool IncomingRequestManager::isHostnameRejected(const QByteArray &hostname) const
{
    QJsonArray blacklist = contacts->identity->settings()->read<QJsonArray>("hostnameBlacklist");
    return blacklist.contains(QString::fromLatin1(hostname));
}

IncomingContactRequest::IncomingContactRequest(IncomingRequestManager *m, const QByteArray &h,
                                                    ContactRequestServer *c)
    : QObject(m), manager(m), connection(c), m_hostname(h)
{
    Q_ASSERT(manager);
    Q_ASSERT(m_hostname.size() == 16);

    qDebug() << "Created contact request from" << m_hostname << (connection ? "with" : "without") << "connection";
}

void IncomingContactRequest::load()
{
    SettingsObject settings(QStringLiteral("contactRequests.%1").arg(QLatin1String(m_hostname)));

    setRemoteSecret(settings.read<Base64Encode>("remoteSecret"));
    setNickname(settings.read("nickname").toString());
    setMessage(settings.read("message").toString());

    m_requestDate = settings.read<QDateTime>("requestDate");
    m_lastRequestDate = settings.read<QDateTime>("lastRequestDate");
}

void IncomingContactRequest::save()
{
    SettingsObject settings(QStringLiteral("contactRequests.%1").arg(QLatin1String(m_hostname)));

    settings.write("remoteSecret", Base64Encode(remoteSecret()));
    settings.write("nickname", nickname());
    settings.write("message", message());

    if (m_requestDate.isNull())
        m_requestDate = m_lastRequestDate = QDateTime::currentDateTime();

    settings.write("requestDate", m_requestDate);
    settings.write("lastRequestDate", m_lastRequestDate);
}

void IncomingContactRequest::renew()
{
    m_lastRequestDate = QDateTime::currentDateTime();
}

void IncomingContactRequest::removeRequest()
{
    SettingsObject settings(QStringLiteral("contactRequests.%1").arg(QLatin1String(m_hostname)));
    settings.undefine();
}

QString IncomingContactRequest::contactId() const
{
    return ContactIDValidator::idFromHostname(hostname());
}

void IncomingContactRequest::setRemoteSecret(const QByteArray &remoteSecret)
{
    Q_ASSERT(remoteSecret.size() == 16);
    m_remoteSecret = remoteSecret;
}

void IncomingContactRequest::setMessage(const QString &message)
{
    m_message = message;
}

void IncomingContactRequest::setNickname(const QString &nickname)
{
    m_nickname = nickname;
    emit nicknameChanged();
}

void IncomingContactRequest::setConnection(ContactRequestServer *c)
{
    if (connection)
    {
        /* New connections replace old ones.. but this should honestly never
         * happen, because the redeliver timeout is far longer. */
        connection.data()->close();
    }

    qDebug() << "Setting new connection for an existing contact request from" << m_hostname;
    connection = c;
    emit hasActiveConnectionChanged();
}

void IncomingContactRequest::accept(ContactUser *user)
{
    qDebug() << "Accepting contact request from" << m_hostname;

    /* Create the contact */
    if (!user)
    {
        Q_ASSERT(!nickname().isEmpty());
        user = manager->contacts->addContact(nickname());
        user->setHostname(QString::fromLatin1(m_hostname));
    }

    user->settings()->write("remoteSecret", Base64Encode(remoteSecret()));

    /* If there is a connection, send the accept message and morph it to a primary connection */
    if (connection)
    {
        connection.data()->sendAccept(user);
        connection = (ContactRequestServer*)0;
    }

    /* Remove the request */
    removeRequest();
    manager->removeRequest(this);

    user->updateStatus();
}

void IncomingContactRequest::reject()
{
    qDebug() << "Rejecting contact request from" << m_hostname;

    /* Send a rejection if there is an active connection */
    if (connection)
        connection.data()->sendRejection();
    /* Remove the request from the config */
    removeRequest();
    /* Blacklist the host to prevent repeat requests */
    manager->addRejectedHost(m_hostname);
    /* Remove the request from the manager */
    manager->removeRequest(this);

    /* Object is now scheduled for deletion */
}
