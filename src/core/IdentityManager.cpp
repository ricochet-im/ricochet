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

#include "IdentityManager.h"
#include "ContactIDValidator.h"
#include "core/OutgoingContactRequest.h"
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

void IdentityManager::addIdentity(UserIdentity *identity)
{
    m_identities.append(identity);
    highestID = qMax(identity->uniqueID, highestID);

    connect(&identity->contacts, SIGNAL(contactAdded(ContactUser*)), SLOT(onContactAdded(ContactUser*)));
    connect(&identity->contacts, SIGNAL(outgoingRequestAdded(OutgoingContactRequest*)),
            SLOT(onOutgoingRequest(OutgoingContactRequest*)));
    connect(&identity->contacts.incomingRequests, SIGNAL(requestAdded(IncomingContactRequest*)),
            SLOT(onIncomingRequest(IncomingContactRequest*)));
    connect(&identity->contacts.incomingRequests, SIGNAL(requestRemoved(IncomingContactRequest*)),
            SLOT(onIncomingRequestRemoved(IncomingContactRequest*)));

    emit identityAdded(identity);
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
        addIdentity(user);
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

    addIdentity(identity);

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

UserIdentity *IdentityManager::lookupUniqueID(int uniqueID) const
{
    for (QList<UserIdentity*>::ConstIterator it = m_identities.begin(); it != m_identities.end(); ++it)
    {
        if ((*it)->uniqueID == uniqueID)
            return *it;
    }

    return 0;
}

void IdentityManager::onContactAdded(ContactUser *user)
{
    emit contactAdded(user, user->identity);
}

void IdentityManager::onOutgoingRequest(OutgoingContactRequest *request)
{
    emit outgoingRequestAdded(request, request->user->identity);
}

void IdentityManager::onIncomingRequest(IncomingContactRequest *request)
{
    emit incomingRequestAdded(request, request->manager->contacts->identity);
}

void IdentityManager::onIncomingRequestRemoved(IncomingContactRequest *request)
{
    emit incomingRequestRemoved(request, request->manager->contacts->identity);
}
