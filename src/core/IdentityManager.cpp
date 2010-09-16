/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#include "IdentityManager.h"
#include "ContactIDValidator.h"
#include <QDebug>

IdentityManager *identityManager = 0;

IdentityManager::IdentityManager(QObject *parent)
    : QObject(parent), highestID(-1)
{
    identityManager = this;

    loadFromSettings();
}

IdentityManager::~IdentityManager()
{
    identityManager = 0;
}

void IdentityManager::loadFromSettings()
{
    config->beginGroup(QLatin1String("identity"));
    QStringList sections = config->childGroups();
    config->endGroup();

    for (QStringList::Iterator it = sections.begin(); it != sections.end(); ++it)
    {
        bool ok = false;
        int id = it->toInt(&ok);
        if (!ok)
            continue;

        UserIdentity *user = new UserIdentity(id, this);
        m_identities.append(user);
        highestID = qMax(id, highestID);
    }

    /* Attempt to convert from old style configs if necessary */
    if (config->contains(QLatin1String("core/serviceDirectory")))
    {
        QString directory = config->value("core/serviceDirectory").toString();
        foreach (UserIdentity *user, m_identities)
        {
            if (user->readSetting("dataDirectory").toString() == directory)
                return;
        }

        qDebug() << "Creating new identity from old single-identity configuration";
        createIdentity(directory);
    }

    if (m_identities.isEmpty())
    {
        /* No identities exist (probably inital run); create one */
        createIdentity();
    }
}

UserIdentity *IdentityManager::createIdentity(const QString &serviceDirectory, const QString &nickname)
{
    UserIdentity *identity = UserIdentity::createIdentity(++highestID, serviceDirectory);
    if (!nickname.isEmpty())
        identity->setNickname(nickname);

    m_identities.append(identity);
    emit identityAdded(identity);

    return identity;
}

UserIdentity *IdentityManager::lookupHostname(const QString &hostname) const
{
    QString ohost = ContactIDValidator::hostnameFromID(hostname);
    if (ohost.isNull())
        ohost = hostname;

    if (!ohost.endsWith(QLatin1String(".onion")))
        ohost.append(QLatin1String(".onion"));

    for (QList<UserIdentity*>::ConstIterator it = m_identities.begin(); it != m_identities.end(); ++it)
    {
        if (ohost.compare((*it)->hostname(), Qt::CaseInsensitive) == 0)
            return *it;
    }

    return 0;
}

UserIdentity *IdentityManager::lookupNickname(const QString &nickname) const
{
    for (QList<UserIdentity*>::ConstIterator it = m_identities.begin(); it != m_identities.end(); ++it)
    {
        if (QString::compare(nickname, (*it)->nickname(), Qt::CaseInsensitive) == 0)
            return *it;
    }

    return 0;
}
