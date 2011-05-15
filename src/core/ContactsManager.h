/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
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

#ifndef CONTACTSMANAGER_H
#define CONTACTSMANAGER_H

#include <QObject>
#include <QList>
#include "ContactUser.h"
#include "IncomingRequestManager.h"

class OutgoingContactRequest;
class UserIdentity;
class IncomingRequestManager;

class ContactsManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactsManager)

    friend class OutgoingContactRequest;

public:
    UserIdentity * const identity;
    IncomingRequestManager incomingRequests;

    explicit ContactsManager(UserIdentity *identity);

    const QList<ContactUser*> &contacts() const { return pContacts; }
    ContactUser *lookupSecret(const QByteArray &secret) const;
    ContactUser *lookupHostname(const QString &hostname) const;
    ContactUser *lookupNickname(const QString &nickname) const;
    ContactUser *lookupUniqueID(int uniqueID) const;

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

#endif // CONTACTSMANAGER_H
