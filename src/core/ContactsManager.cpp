/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
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

#include "ContactsManager.h"
#include "IncomingRequestManager.h"
#include "OutgoingContactRequest.h"
#include "ContactIDValidator.h"
#include "ConversationModel.h"
#include <QStringList>
#include <QDebug>

ContactsManager *contactsManager = 0;

ContactsManager::ContactsManager(UserIdentity *id)
    : identity(id), incomingRequests(this), highestID(-1)
{
    contactsManager = this;
}

void ContactsManager::loadFromSettings()
{
    SettingsObject settings(QStringLiteral("contacts"));
    foreach (const QString &key, settings.data().keys())
    {
        bool ok = false;
        int id = key.toInt(&ok);
        if (!ok)
        {
            qWarning() << "Ignoring contact" << key << " with a non-integer ID";
            continue;
        }

    	ContactUser *user = new ContactUser(identity, id, this);
        connectSignals(user);
    	pContacts.append(user);
        highestID = qMax(id, highestID);
    }

    incomingRequests.loadRequests();
}

ContactUser *ContactsManager::addContact(const QString &nickname)
{
    Q_ASSERT(!nickname.isEmpty());

    highestID++;
    ContactUser *user = ContactUser::addNewContact(identity, highestID);
    user->setParent(this);
    user->setNickname(nickname);
    connectSignals(user);

    qDebug() << "Added new contact" << nickname << "with ID" << user->uniqueID;

    pContacts.append(user);
    emit contactAdded(user);

    return user;
}

void ContactsManager::connectSignals(ContactUser *user)
{
    connect(user, SIGNAL(contactDeleted(ContactUser*)), SLOT(contactDeleted(ContactUser*)));
    connect(user->conversation(), &ConversationModel::unreadCountChanged, this, &ContactsManager::onUnreadCountChanged);
    connect(user, &ContactUser::statusChanged, [this,user]() { emit contactStatusChanged(user, user->status()); });
}

ContactUser *ContactsManager::createContactRequest(const QString &contactid, const QString &nickname,
                                                   const QString &myNickname, const QString &message)
{
    QString hostname = ContactIDValidator::hostnameFromID(contactid);
    if (hostname.isEmpty() || lookupHostname(contactid) || lookupNickname(nickname))
    {
        return 0;
    }

    bool b = blockSignals(true);
    ContactUser *user = addContact(nickname);
    blockSignals(b);
    if (!user)
        return user;
    user->setHostname(ContactIDValidator::hostnameFromID(contactid));

    OutgoingContactRequest::createNewRequest(user, myNickname, message);

    /* Signal deferred from addContact to avoid changing the status immediately */
    Q_ASSERT(user->status() == ContactUser::RequestPending);
    emit contactAdded(user);
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
        if (secret == (*it)->settings()->read<Base64Encode>("localSecret"))
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

void ContactsManager::onUnreadCountChanged()
{
    ConversationModel *model = qobject_cast<ConversationModel*>(sender());
    Q_ASSERT(model);
    if (!model)
        return;
    ContactUser *user = model->contact();

    emit unreadCountChanged(user, model->unreadCount());
}

