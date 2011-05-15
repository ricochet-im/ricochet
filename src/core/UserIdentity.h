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

#ifndef USERIDENTITY_H
#define USERIDENTITY_H

#include "main.h"
#include "ContactsManager.h"
#include <QObject>
#include <QPixmapCache>
#include <QMetaType>

class QPixmap;
class QImage;

namespace Tor
{
    class HiddenService;
}

class IncomingSocket;

class UserIdentity : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserIdentity)

    friend class IdentityManager;

    Q_PROPERTY(int uniqueID READ getUniqueID CONSTANT)
    Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
    Q_PROPERTY(QString contactID READ contactID NOTIFY contactIDChanged)

public:
    const int uniqueID;
    ContactsManager contacts;

    explicit UserIdentity(int uniqueID, QObject *parent = 0);

    /* Properties */
    int getUniqueID() const { return uniqueID; }
    const QString &nickname() const { return m_nickname; }
    /* Hostname is .onion format, like ContactUser */
    QString hostname() const;
    QString contactID() const;

    void setNickname(const QString &nickname);
    void setAvatar(QImage image);

    /* State */
    bool isServiceOnline() const;
    bool isServicePublished() const;

    /* Settings API */
    QVariant readSetting(const QString &key, const QVariant &defaultValue = QVariant()) const;
    QVariant readSetting(const char *key, const QVariant &defaultValue = QVariant()) const
    {
        return readSetting(QLatin1String(key), defaultValue);
    }

    void writeSetting(const QString &key, const QVariant &value);
    void writeSetting(const char *key, const QVariant &value)
    {
        writeSetting(QLatin1String(key), value);
    }

    void removeSetting(const QString &key);
    void removeSetting(const char *key)
    {
        removeSetting(QLatin1String(key));
    }

signals:
    void statusChanged();
    void contactIDChanged(); // only possible during creation
    void nicknameChanged();
    void settingsChanged(const QString &key);

private slots:
    void onStatusChanged(int newStatus, int oldStatus);

private:
    QString m_nickname;

    Tor::HiddenService *hiddenService;
    IncomingSocket *incomingSocket;

    static UserIdentity *createIdentity(int uniqueID, const QString &dataDirectory = QString());
};

Q_DECLARE_METATYPE(UserIdentity*)

#endif // USERIDENTITY_H
