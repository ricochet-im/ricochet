#include "main.h"
#include "OutgoingRequestManager.h"
#include "ContactsManager.h"
#include "ContactUser.h"
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
        if ((*it)->readSetting(QLatin1String("addRequest")).toBool())
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

    user->writeSetting(QLatin1String("addRequest"), true);
    user->writeSetting(QLatin1String("requestNickname"), myNickname);
    user->writeSetting(QLatin1String("requestMessage"), message);

    startRequest(user);
}

void OutgoingRequestManager::startRequest(ContactUser *user)
{
    Q_ASSERT(!users.contains(user));

    qDebug() << "Starting outgoing contact request for" << user->uniqueID;

    ContactRequestClient *client = new ContactRequestClient(user);
    users.insert(user, client);

    connect(client, SIGNAL(responseChanged(int)), this, SLOT(handleResponse(int)));

    client->setMyNickname(user->readSetting(QLatin1String("requestNickname")).toString());
    client->setMessage(user->readSetting(QLatin1String("requestMessage")).toString());
    client->sendRequest();
}

void OutgoingRequestManager::handleResponse(int response)
{
    ContactRequestClient *client = qobject_cast<ContactRequestClient*>(sender());
    if (!client)
        return;

    qDebug() << "Received response" << response << "for contact request to" << client->user->uniqueID;
}
