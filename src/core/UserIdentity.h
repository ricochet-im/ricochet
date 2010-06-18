/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
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
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

#ifndef USERIDENTITY_H
#define USERIDENTITY_H

#include "main.h"
#include <QObject>
#include <QPixmapCache>
#include <QMetaType>

class QPixmap;
class QImage;

namespace Tor
{
    class HiddenService;
}

class UserIdentity : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(UserIdentity)

public:
    const int uniqueID;

    explicit UserIdentity(int uniqueID, QObject *parent = 0);

    /* Properties */
    const QString &nickname() const { return m_nickname; }
    /* Hostname is .onion format, like ContactUser */
    QString hostname() const;
    QPixmap avatar(AvatarSize size);

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

private:
    QString m_nickname;
    QPixmapCache::Key cachedAvatar[2];

    Tor::HiddenService *hiddenService;
};

Q_DECLARE_METATYPE(UserIdentity*)

#endif // USERIDENTITY_H
