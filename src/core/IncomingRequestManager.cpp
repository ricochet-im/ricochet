#include "main.h"
#include "IncomingRequestManager.h"
#include "ContactsManager.h"
#include "protocol/ContactRequestServer.h"
#include <QDateTime>

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

void IncomingRequestManager::addRequest(const QByteArray &hostname, ContactRequestServer *connection,
                                        const QString &nickname, const QString &message)
{
    IncomingContactRequest *request = requestFromHostname(hostname);
    if (request)
    {
        /* Update the existing request */
        request->setConnection(connection);
        request->setNickname(nickname);
        request->setMessage(message);
        request->save();
        return;
    }

    /* Create a new request */
    request = new IncomingContactRequest(this, hostname, connection);
    request->setNickname(nickname);
    request->setMessage(message);

    request->save();

    m_requests.append(request);
    emit requestAdded(request);
}

IncomingContactRequest::IncomingContactRequest(IncomingRequestManager *m, const QByteArray &h,
                                                    ContactRequestServer *c)
    : QObject(m), manager(m), hostname(h), connection(c)
{
    Q_ASSERT(manager);
    Q_ASSERT(hostname.size() == 16);
}

void IncomingContactRequest::load()
{
    config->beginGroup(QLatin1String("contactRequests/") + QString::fromLatin1(hostname));
    setNickname(config->value(QLatin1String("nickname")).toString());
    setMessage(config->value(QLatin1String("message")).toString());
    config->endGroup();
}

void IncomingContactRequest::save()
{
    config->beginGroup(QLatin1String("contactRequests/") + QString::fromLatin1(hostname));
    config->setValue(QLatin1String("nickname"), nickname());
    config->setValue(QLatin1String("message"), message());
    config->endGroup();
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
        connection->close();
    }

    connection = c;
}

QDateTime IncomingContactRequest::requestDate() const
{
    return QDateTime();
}

QDateTime IncomingContactRequest::lastRequestDate() const
{
    return QDateTime();
}

void IncomingContactRequest::accept()
{

}

void IncomingContactRequest::reject()
{

}
