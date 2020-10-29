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

#ifndef IDENTITYMANAGER_H
#define IDENTITYMANAGER_H

// TODO: this needs to go entirely, we do not have multiple simultaneous UserIdentity objects
class IdentityManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IdentityManager)

public:
    // serviceID : string ED25519-V3 keyblob pulled from config.json, or empty string to create one
    explicit IdentityManager(const QString& serviceID, QObject *parent = 0);
    ~IdentityManager();

    const QList<class UserIdentity*> &identities() const { return m_identities; }
    class UserIdentity *lookupNickname(const QString &nickname) const;
    class UserIdentity *lookupHostname(const QString &hostname) const;
    class UserIdentity *lookupUniqueID(int uniqueID) const;

    class UserIdentity *createIdentity();

signals:
    void identityAdded(class UserIdentity *identity);
    void contactAdded(class ContactUser *user, class UserIdentity *identity);
    void contactDeleted(class ContactUser *user, class UserIdentity *identity);
    void outgoingRequestAdded(class OutgoingContactRequest *request, class UserIdentity *identity);
    void incomingRequestAdded(class IncomingContactRequest *request, class UserIdentity *identity);
    void incomingRequestRemoved(class IncomingContactRequest *request, class UserIdentity *identity);

private slots:
    void onContactAdded(class ContactUser *user);
    void onOutgoingRequest(class OutgoingContactRequest *request);
    void onIncomingRequest(class IncomingContactRequest *request);
    void onIncomingRequestRemoved(class IncomingContactRequest *request);

private:
    QList<class UserIdentity*> m_identities;
    int highestID;

    void addIdentity(class UserIdentity *identity);
};

extern class IdentityManager* identityManager;

#endif // IDENTITYMANAGER_H
