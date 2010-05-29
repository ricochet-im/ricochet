#include "main.h"
#include "OutgoingRequestManager.h"
#include "ContactsManager.h"
#include "ContactUser.h"
#include "IncomingRequestManager.h"
#include "protocol/ContactRequestClient.h"
#include "tor/TorControlManager.h"

OutgoingRequestManager::OutgoingRequestManager(ContactsManager *c)
    : QObject(c), contacts(c)
{
    connect(torManager, SIGNAL(socksReady()), this, SLOT(loadRequests()));
}

void OutgoingRequestManager::loadRequests()
{
    QList<ContactUser*> contactList = contacts->contacts();
    for (QList<ContactUser*>::iterator it = contactList.begin(); it != contactList.end(); ++it)
    {
        if ((*it)->readSetting(QLatin1String("request/isRequest")).toBool())
        {
            if (users.contains(*it))
                continue;

            startRequest(*it);
        }
    }
}

void OutgoingRequestManager::addNewRequest(ContactUser *user, const QString &myNickname, const QString &message)
{
    Q_ASSERT(!user->isConnected());
    if (users.contains(user))
        return;

    /* Check if there is an existing incoming request that matches this one; if so, treat this as accepted
     * automatically and accept that incoming request for this user */
    QByteArray hostname = user->hostname().left(16).toLatin1();
    IncomingContactRequest *incomingReq = contactsManager->incomingRequests->requestFromHostname(hostname);
    if (incomingReq)
    {
        qDebug() << "Automatically accepting an incoming contact request matching a newly created outgoing request";

        acceptRequest(user);
        incomingReq->accept(user);
        return;
    }

    user->writeSetting(QLatin1String("request/isRequest"), true);
    user->writeSetting(QLatin1String("request/myNickname"), myNickname);
    user->writeSetting(QLatin1String("request/message"), message);

    startRequest(user);
}

void OutgoingRequestManager::removeRequest(ContactUser *user)
{
    /* Clear the list entry and disconnect from client signals */
    QMap<ContactUser*,ContactRequestClient*>::iterator it = users.find(user);
    if (it == users.end())
        return;

    (*it)->disconnect(this);
    if ((*it)->response() <= ContactRequestClient::Acknowledged)
        (*it)->close();

    (*it)->deleteLater();
    users.erase(it);

    /* Clear the request settings */
    config->beginGroup(QString::fromLatin1("contacts/%1/request").arg(user->uniqueID));
    config->remove("");
    config->endGroup();
}

void OutgoingRequestManager::startRequest(ContactUser *user)
{
    Q_ASSERT(!users.contains(user));

    qDebug() << "Starting outgoing contact request for" << user->uniqueID;

    ContactRequestClient *client = new ContactRequestClient(user);
    users.insert(user, client);

    connect(client, SIGNAL(accepted()), SLOT(requestAccepted()));
    connect(client, SIGNAL(rejected(int)), SLOT(requestRejected(int)));

    client->setMyNickname(user->readSetting("request/myNickname").toString());
    client->setMessage(user->readSetting("request/message").toString());
    client->sendRequest();
}

void OutgoingRequestManager::acceptRequest(ContactUser *user)
{
    /* Remove the request data */
    removeRequest(user);

    /* Update the user status */
    user->updateStatusLine();

    /* We don't have enough information to make a connection yet (no remoteSecret);
     * if there is an active request connection, it will turn into a primary and the
     * secret will be automatically received there. Otherwise, we must wait for the
     * peer to connect to us. */
}

void OutgoingRequestManager::rejectRequest(ContactUser *user, const QString &reason)
{
    qFatal("OutgoingRequestManager::rejectRequest is not implemented");
}

void OutgoingRequestManager::requestAccepted()
{
    ContactRequestClient *client = qobject_cast<ContactRequestClient*>(sender());
    if (!client)
        return;

    acceptRequest(client->user);
}

void OutgoingRequestManager::requestRejected(int reason)
{
    Q_UNUSED(reason);

    ContactRequestClient *client = qobject_cast<ContactRequestClient*>(sender());
    if (!client)
        return;

    rejectRequest(client->user, QString());
}
