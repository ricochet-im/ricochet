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
#include "protocol/ChatChannel.h"

ContactsManager *contactsManager = 0;

ContactsManager::ContactsManager(UserIdentity *id)
    : identity(id), incomingRequests(this)
{
    contactsManager = this;
}

void ContactsManager::loadFromSettings(const QVector<QString>& contactHostnames)
{
    for(const QString& hostname : contactHostnames)
    {
        ContactUser *user = new ContactUser(identity, hostname, this);
        connectSignals(user);
        pContacts.append(user);
        emit contactAdded(user);
    }

    incomingRequests.loadRequests();
}

ContactUser *ContactsManager::addContact(const QString& hostname, const QString &nickname)
{
    Q_ASSERT(!nickname.isEmpty());

    ContactUser *user = ContactUser::addNewContact(identity, hostname);
    user->setParent(this);
    user->setNickname(nickname);
    connectSignals(user);

    qDebug() << "Added new contact" << hostname << "(" << nickname << ")";

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
    const auto contactHostname = ContactIDValidator::hostnameFromID(contactid);
    ContactUser *user = addContact(contactHostname, nickname);
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

void ContactsManager::onUnreadCountChanged()
{
    ConversationModel *model = qobject_cast<ConversationModel*>(sender());
    Q_ASSERT(model);
    if (!model)
        return;
    ContactUser *user = model->contact();

    emit unreadCountChanged(user, model->unreadCount());

#ifdef Q_OS_MAC
    int unread = globalUnreadCount();
    QtMac::setBadgeLabelText(unread == 0 ? QString() : QString::number(unread));
#endif
}

int ContactsManager::globalUnreadCount() const
{
    int re = 0;
    foreach (ContactUser *u, pContacts) {
        if (u->conversation())
            re += u->conversation()->unreadCount();
    }
    return re;
}

