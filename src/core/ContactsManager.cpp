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
#include "ContactsManager.h"
#include "IncomingRequestManager.h"
#include "OutgoingContactRequest.h"
#include "ContactIDValidator.h"
#include <QStringList>

ContactsManager *contactsManager = 0;

ContactsManager::ContactsManager()
    : incomingRequests(new IncomingRequestManager(this)),
      highestID(-1)
{
    contactsManager = this;

    loadFromSettings();
    incomingRequests->loadRequests();
}

void ContactsManager::loadFromSettings()
{
    config->beginGroup(QLatin1String("contacts"));
    QStringList sections = config->childGroups();
    config->endGroup();

    for (QStringList::Iterator it = sections.begin(); it != sections.end(); ++it)
    {
        bool ok = false;
        int id = it->toInt(&ok);
        if (!ok)
        {
            qWarning() << "Ignoring contact" << *it << "with a non-integer ID";
            continue;
        }

    	ContactUser *user = new ContactUser(id, this);
    	pContacts.append(user);
        highestID = qMax(id, highestID);
    }
}

ContactUser *ContactsManager::addContact(const QString &nickname)
{
    Q_ASSERT(!nickname.isEmpty());

    highestID++;
    ContactUser *user = ContactUser::addNewContact(highestID);
    user->setParent(this);
    user->setNickname(nickname);

    qDebug() << "Added new contact" << nickname << "with ID" << user->uniqueID;

    pContacts.append(user);
    emit contactAdded(user);

    return user;
}

ContactUser *ContactsManager::lookupSecret(const QByteArray &secret) const
{
    Q_ASSERT(secret.size() == 16);

    for (QList<ContactUser*>::ConstIterator it = pContacts.begin(); it != pContacts.end(); ++it)
    {
        if ((*it)->readSetting("localSecret") == secret)
            return *it;
    }

    return 0;
}

ContactUser *ContactsManager::lookupHostname(const QString &hostname) const
{
    QString ohost = ContactIDValidator::hostnameFromID(hostname);
    if (ohost.isNull())
        ohost = hostname;

    if (!ohost.endsWith(QLatin1String(".onion")))
        ohost.append(QLatin1String(".onion"));

    for (QList<ContactUser*>::ConstIterator it = pContacts.begin(); it != pContacts.end(); ++it)
    {
        if (ohost.compare((*it)->hostname(), Qt::CaseInsensitive) == 0)
            return *it;
    }

    return 0;
}

ContactUser *ContactsManager::lookupNickname(const QString &nickname) const
{
    for (QList<ContactUser*>::ConstIterator it = pContacts.begin(); it != pContacts.end(); ++it)
    {
        if (QString::compare(nickname, (*it)->nickname(), Qt::CaseInsensitive) == 0)
            return *it;
    }

    return 0;
}

void ContactsManager::connectToAll()
{
    qDebug() << "Attempting connections to all contacts";
    for (QList<ContactUser*>::ConstIterator it = pContacts.begin(); it != pContacts.end(); ++it)
        (*it)->conn()->connectPrimary();
}
