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

#include "main.h"
#include "UserIdentity.h"
#include "tor/TorControlManager.h"
#include "tor/HiddenService.h"
#include "protocol/IncomingSocket.h"
#include <QImage>
#include <QPixmap>
#include <QPixmapCache>
#include <QBuffer>

/* Imported from main.cpp; will be obsolete when proper multiple identity support is done */
extern IncomingSocket *incomingSocket;

UserIdentity::UserIdentity(int id, QObject *parent)
    : QObject(parent), uniqueID(id)
{
    m_nickname = readSetting("nickname", tr("Me")).toString();

    QString dir = readSetting("dataDirectory", QLatin1String("data-") + QString::number(uniqueID)).toString();
    hiddenService = new Tor::HiddenService(dir, this);
    connect(hiddenService, SIGNAL(statusChanged(int,int)), SIGNAL(statusChanged()));

    if (hiddenService->status() == Tor::HiddenService::NotCreated)
    {
        qWarning() << "Hidden service data for identity" << uniqueID << "in" << dir << "does not exist";
        delete hiddenService;
        hiddenService = 0;
    }
    else
    {
        hiddenService->addTarget(80, incomingSocket->serverAddress(), incomingSocket->serverPort());
        torManager->addHiddenService(hiddenService);
    }
}

QVariant UserIdentity::readSetting(const QString &key, const QVariant &defaultValue) const
{
    return config->value(QString::fromLatin1("identity/%1/%2").arg(uniqueID).arg(key), defaultValue);
}

void UserIdentity::writeSetting(const QString &key, const QVariant &value)
{
    config->setValue(QString::fromLatin1("identity/%1/%2").arg(uniqueID).arg(key), value);
    emit settingsChanged(key);
}

void UserIdentity::removeSetting(const QString &key)
{
    config->remove(QString::fromLatin1("identity/%1/%2").arg(uniqueID).arg(key));
    emit settingsChanged(key);
}

QString UserIdentity::hostname() const
{
    return hiddenService ? hiddenService->hostname() : QString();
}

void UserIdentity::setNickname(const QString &nick)
{
    if (nick == m_nickname)
        return;

    m_nickname = nick;
    writeSetting("nickname", nick);
}

QPixmap UserIdentity::avatar(AvatarSize size)
{
    QPixmap re;
    if (QPixmapCache::find(cachedAvatar[size], &re))
        return re;

    QByteArray data = readSetting((size == TinyAvatar) ? "avatar-tiny" : "avatar").toByteArray();
    re.loadFromData(data);

    cachedAvatar[size] = QPixmapCache::insert(re);
    return re;
}

void UserIdentity::setAvatar(QImage image)
{
    if (image.width() > 160 || image.height() > 160)
        image = image.scaled(QSize(160, 160), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (!image.isNull())
    {
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        if (image.save(&buffer, "jpeg", 100))
        {
            writeSetting("avatar", buffer.buffer());

            QImage tiny = image.scaled(QSize(35, 35), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            buffer.close();
            buffer.open(QBuffer::ReadWrite);
            if (tiny.save(&buffer, "jpeg", 100))
                writeSetting("avatar-tiny", buffer.buffer());
            else
                image = QImage();
        }
        else
            image = QImage();
    }

    if (image.isNull())
    {
        removeSetting("avatar");
        removeSetting("avatar-tiny");
    }

    for (int i = 0; i < 2; ++i)
        QPixmapCache::remove(cachedAvatar[i]);
}

bool UserIdentity::isServiceOnline() const
{
    return hiddenService->status() == Tor::HiddenService::Online;
}

bool UserIdentity::isServicePublished() const
{
    return hiddenService->status() >= Tor::HiddenService::Published;
}
