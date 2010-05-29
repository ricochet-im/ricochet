#include "main.h"
#include "IncomingRequestManager.h"
#include "ContactsManager.h"
#include "protocol/ContactRequestServer.h"

IncomingRequestManager::IncomingRequestManager(ContactsManager *c)
    : QObject(c), contacts(c)
{
}

void IncomingRequestManager::loadRequests()
{
    config->beginGroup(QLatin1String("contactRequests"));
    QStringList contacts = config->childGroups();
    config->endGroup();

    for (QStringList::ConstIterator it = contacts.begin(); it != contacts.end(); ++it)
    {
        IncomingContactRequest *request = new IncomingContactRequest(this, it->toLatin1());
        request->load();

        m_requests.append(request);
        emit requestAdded(request);
    }
}

IncomingContactRequest *IncomingRequestManager::requestFromHostname(const QByteArray &hostname)
{
    Q_ASSERT(!hostname.endsWith(".onion"));
    Q_ASSERT(hostname == hostname.toLower());

    for (QList<IncomingContactRequest*>::ConstIterator it = m_requests.begin(); it != m_requests.end(); ++it)
        if ((*it)->hostname == hostname)
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

    IncomingContactRequest *request = requestFromHostname(hostname);
    if (request)
    {
        /* Update the existing request */
        request->setConnection(connection);
        request->setRemoteSecret(connSecret);
        request->setNickname(nickname);
        request->setMessage(message);
        request->renew();
        request->save();
        return;
    }

    /* Create a new request */
    request = new IncomingContactRequest(this, hostname, connection);
    request->setRemoteSecret(connSecret);
    request->setNickname(nickname);
    request->setMessage(message);

    request->save();

    m_requests.append(request);
    emit requestAdded(request);
}

void IncomingRequestManager::removeRequest(IncomingContactRequest *request)
{
    if (m_requests.removeOne(request))
        emit requestRemoved(request);

    request->deleteLater();
}

void IncomingRequestManager::addRejectedHost(const QByteArray &hostname)
{
    QStringList hosts = rejectedHosts();
    hosts.append(QString::fromLatin1(hostname));
    config->setValue("core/hostnameBlacklist", QVariant::fromValue(hosts));
}

bool IncomingRequestManager::isHostnameRejected(const QByteArray &hostname) const
{
    return rejectedHosts().contains(QString::fromLatin1(hostname));
}

QStringList IncomingRequestManager::rejectedHosts() const
{
    return config->value("core/hostnameBlacklist").value<QStringList>();
}

IncomingContactRequest::IncomingContactRequest(IncomingRequestManager *m, const QByteArray &h,
                                                    ContactRequestServer *c)
    : QObject(m), manager(m), hostname(h), connection(c)
{
    Q_ASSERT(manager);
    Q_ASSERT(hostname.size() == 16);

    qDebug() << "Created contact request from" << hostname << (connection ? "with" : "without") << "connection";
}

void IncomingContactRequest::load()
{
    config->beginGroup(QLatin1String("contactRequests/") + QString::fromLatin1(hostname));

    setRemoteSecret(config->value(QLatin1String("remoteSecret")).toByteArray());
    setNickname(config->value(QLatin1String("nickname")).toString());
    setMessage(config->value(QLatin1String("message")).toString());

    m_requestDate = config->value(QLatin1String("requestDate")).toDateTime();
    m_lastRequestDate = config->value(QLatin1String("lastRequestDate")).toDateTime();

    config->endGroup();
}

void IncomingContactRequest::save()
{
    config->beginGroup(QLatin1String("contactRequests/") + QString::fromLatin1(hostname));

    config->setValue(QLatin1String("remoteSecret"), remoteSecret());
    config->setValue(QLatin1String("nickname"), nickname());
    config->setValue(QLatin1String("message"), message());

    if (m_requestDate.isNull())
        m_requestDate = m_lastRequestDate = QDateTime::currentDateTime();

    config->setValue(QLatin1String("requestDate"), m_requestDate);
    config->setValue(QLatin1String("lastRequestDate"), m_lastRequestDate);

    config->endGroup();
}

void IncomingContactRequest::renew()
{
    m_lastRequestDate = QDateTime::currentDateTime();
}

void IncomingContactRequest::removeRequest()
{
    /* Remove from config */
    config->beginGroup(QLatin1String("contactRequests/") + QString::fromLatin1(hostname));
    config->remove("");
    config->endGroup();
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
}

void IncomingContactRequest::setConnection(ContactRequestServer *c)
{
    if (connection)
    {
        /* New connections replace old ones.. but this should honestly never
         * happen, because the redeliver timeout is far longer. */
        connection.data()->close();
    }

    qDebug() << "Setting new connection for an existing contact request from" << hostname;
    connection = c;
}

void IncomingContactRequest::accept()
{
    qDebug() << "Accepting contact request from" << hostname;
    qFatal("Not implemented");
}

void IncomingContactRequest::reject()
{
    qDebug() << "Rejecting contact request from" << hostname;

    /* Send a rejection if there is an active connection */
    if (connection)
        connection.data()->sendRejection();
    /* Remove the request from the config */
    removeRequest();
    /* Blacklist the host to prevent repeat requests */
    manager->addRejectedHost(hostname);
    /* Remove the request from the manager */
    manager->removeRequest(this);

    /* Object is now scheduled for deletion */
}
