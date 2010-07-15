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

#ifndef CONTACTSMANAGER_H
#define CONTACTSMANAGER_H

#include <QObject>
#include <QList>
#include "ContactUser.h"

class OutgoingContactRequest;

class ContactsManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactsManager)

    friend class OutgoingContactRequest;

public:
    class IncomingRequestManager * const incomingRequests;

    explicit ContactsManager();

    const QList<ContactUser*> &contacts() const { return pContacts; }
    ContactUser *lookupSecret(const QByteArray &secret) const;
    ContactUser *lookupHostname(const QString &hostname) const;
    ContactUser *lookupNickname(const QString &nickname) const;

    ContactUser *addContact(const QString &nickname);

    static QString hostnameFromID(const QString &ID);

public slots:
    void connectToAll();

signals:
    void contactAdded(ContactUser *user);
    void outgoingRequestAdded(OutgoingContactRequest *request);

private slots:
    void contactDeleted(ContactUser *user);

private:
    QList<ContactUser*> pContacts;
    int highestID;

    void loadFromSettings();
};

extern ContactsManager *contactsManager;

#endif // CONTACTSMANAGER_H
