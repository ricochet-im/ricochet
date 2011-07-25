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

#include "main.h"
#include "ContactsManager.h"
#include "IncomingRequestManager.h"
#include "OutgoingContactRequest.h"
#include "ContactIDValidator.h"
#include <QStringList>
#include <QDebug>

ContactsManager *contactsManager = 0;

ContactsManager::ContactsManager(UserIdentity *id)
    : identity(id), incomingRequests(this), highestID(-1)
{
    contactsManager = this;

    loadFromSettings();
    incomingRequests.loadRequests();
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
            qWarning("Ignoring contact %s with a non-integer ID", qPrintable(*it));
            continue;
        }

    	ContactUser *user = new ContactUser(identity, id, this);
        connect(user, SIGNAL(contactDeleted(ContactUser*)), SLOT(contactDeleted(ContactUser*)));
    	pContacts.append(user);
        highestID = qMax(id, highestID);
    }
}

ContactUser *ContactsManager::addContact(const QString &nickname)
{
    Q_ASSERT(!nickname.isEmpty());

    highestID++;
    ContactUser *user = ContactUser::addNewContact(identity, highestID);
    user->setParent(this);
    user->setNickname(nickname);
    connect(user, SIGNAL(contactDeleted(ContactUser*)), SLOT(contactDeleted(ContactUser*)));

    qDebug() << "Added new contact" << nickname << "with ID" << user->uniqueID;

    pContacts.append(user);
    emit contactAdded(user);

    return user;
}

ContactUser *ContactsManager::createContactRequest(const QString &contactid, const QString &nickname,
                                                   const QString &myNickname, const QString &message)
{
    if (!ContactIDValidator::isValidID(contactid) || lookupHostname(contactid) ||
        lookupNickname(nickname) || message.isEmpty())
    {
        return 0;
    }

    ContactUser *user = addContact(nickname);
    if (!user)
        return user;
    user->setHostname(ContactIDValidator::hostnameFromID(contactid));

    OutgoingContactRequest::createNewRequest(user, myNickname, message);
    return user;
}

void ContactsManager::contactDeleted(ContactUser *user)
{
    pContacts.removeOne(user);
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

ContactUser *ContactsManager::lookupUniqueID(int uniqueID) const
{
    for (QList<ContactUser*>::ConstIterator it = pContacts.begin(); it != pContacts.end(); ++it)
    {
        if ((*it)->uniqueID == uniqueID)
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
