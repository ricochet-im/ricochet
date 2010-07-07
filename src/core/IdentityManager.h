/* Torsion - http://github.com/special/torsion
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

#ifndef IDENTITYMANAGER_H
#define IDENTITYMANAGER_H

#include <QObject>
#include "UserIdentity.h"

class IdentityManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IdentityManager)

public:
    explicit IdentityManager(QObject *parent = 0);
    ~IdentityManager();

    const QList<UserIdentity*> &identities() const { return m_identities; }
    UserIdentity *lookupNickname(const QString &nickname) const;
    UserIdentity *lookupHostname(const QString &hostname) const;

    UserIdentity *createIdentity(const QString &serviceDirectory = QString(), const QString &nickname = QString());

signals:
    void identityAdded(UserIdentity *identity);

private:
    QList<UserIdentity*> m_identities;
    int highestID;

    void loadFromSettings();
};

extern IdentityManager *identityManager;

#endif // IDENTITYMANAGER_H
