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

#ifndef CONTACTUSER_H
#define CONTACTUSER_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QHash>
#include <QPixmapCache>
#include <QMetaType>
#include <QVariant>
#include "protocol/ProtocolManager.h"

class ContactUser : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactUser)

    Q_PROPERTY(QString nickname READ nickname WRITE setNickname STORED true)

public:
    enum AvatarSize
    {
        FullAvatar, /* 160x160 */
        TinyAvatar /* 35x35 */
    };

    const QString uniqueID;

    explicit ContactUser(const QString &uniqueID, QObject *parent = 0);

    ProtocolManager *conn() const { return pConn; }
    bool isConnected() const { return pConn->isPrimaryConnected(); }

    const QString &nickname() const { return pNickname; }
    QString notesText() const;
    QPixmap avatar(AvatarSize size);

    QString statusLine() const;

    QVariant readSetting(const QString &key, const QVariant &defaultValue = QVariant()) const;
    void writeSetting(const QString &key, const QVariant &value);

public slots:
    void setNickname(const QString &nickname);
    void setAvatar(QImage image);
    void setNotesText(const QString &notesText);

    void updateStatusLine();

signals:
    void connected();
    void disconnected();

    void statusLineChanged();

private slots:
    void onConnected();
    void onDisconnected();

private:
    ProtocolManager *pConn;
    QString pNickname;
    QPixmapCache::Key cachedAvatar[2];

    void loadSettings();
};

Q_DECLARE_METATYPE(ContactUser*)

#endif // CONTACTUSER_H
