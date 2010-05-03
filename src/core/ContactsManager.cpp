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
#include <QStringList>

ContactsManager *contactsManager = 0;

ContactsManager::ContactsManager()
{
    loadFromSettings();
}

void ContactsManager::loadFromSettings()
{
    config->beginGroup(QString("contacts"));
    QStringList sections = config->childGroups();
    config->endGroup();

    for (QStringList::Iterator it = sections.begin(); it != sections.end(); ++it)
    {
        ContactUser *user = new ContactUser(*it, this);
        pContacts.append(user);
    }
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

void ContactsManager::connectToAll()
{
    qDebug() << "Attempting connections to all contacts";
    for (QList<ContactUser*>::ConstIterator it = pContacts.begin(); it != pContacts.end(); ++it)
        (*it)->conn()->connectPrimary();
}
