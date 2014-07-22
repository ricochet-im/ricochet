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

#ifndef USERIDENTITY_H
#define USERIDENTITY_H

#include "ContactsManager.h"
#include <QObject>
#include <QMetaType>

namespace Tor
{
    class HiddenService;
}

class IncomingSocket;

/* UserIdentity represents the local identity offered by the user.
 *
 * In particular, it represents the published hidden service, and
 * theoretically holds the list of contacts.
 *
 * At present, implementation (and settings) assumes that there is
 * only one identity, but some code is confusingly written to allow
 * for several.
 */
class UserIdentity : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserIdentity)

    friend class IdentityManager;

    Q_PROPERTY(int uniqueID READ getUniqueID CONSTANT)
    Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
    Q_PROPERTY(QString contactID READ contactID NOTIFY contactIDChanged)
    Q_PROPERTY(bool isOnline READ isServiceOnline NOTIFY statusChanged)
    Q_PROPERTY(bool isPublished READ isServicePublished NOTIFY statusChanged)
    Q_PROPERTY(ContactsManager *contacts READ getContacts CONSTANT)
    Q_PROPERTY(SettingsObject *settings READ settings CONSTANT)

public:
    const int uniqueID;
    ContactsManager contacts;

    explicit UserIdentity(int uniqueID, QObject *parent = 0);

    /* Properties */
    int getUniqueID() const { return uniqueID; }
    QString nickname() const;
    /* Hostname is .onion format, like ContactUser */
    QString hostname() const;
    QString contactID() const;

    ContactsManager *getContacts() { return &contacts; }

    void setNickname(const QString &nickname);

    /* State */
    bool isServiceOnline() const;
    bool isServicePublished() const;
    Tor::HiddenService *hiddenService() const { return m_hiddenService; }

    SettingsObject *settings();

signals:
    void statusChanged();
    void contactIDChanged(); // only possible during creation
    void nicknameChanged();
    void settingsChanged(const QString &key);

private slots:
    void onStatusChanged(int newStatus, int oldStatus);
    void onSettingsModified(const QString &key, const QJsonValue &value);

private:
    SettingsObject *m_settings;
    Tor::HiddenService *m_hiddenService;
    IncomingSocket *incomingSocket;

    static UserIdentity *createIdentity(int uniqueID, const QString &dataDirectory = QString());
};

Q_DECLARE_METATYPE(UserIdentity*)

#endif // USERIDENTITY_H
