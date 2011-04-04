/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#include "main.h"
#include "UserIdentity.h"
#include "tor/TorControlManager.h"
#include "tor/HiddenService.h"
#include "protocol/IncomingSocket.h"
#include "core/ContactIDValidator.h"
#include <QImage>
#include <QPixmap>
#include <QPixmapCache>
#include <QBuffer>
#include <QDir>

UserIdentity::UserIdentity(int id, QObject *parent)
    : QObject(parent), uniqueID(id), contacts(this), incomingSocket(0)
{
    m_nickname = readSetting("nickname", tr("Me")).toString();

    QString dir = readSetting("dataDirectory", QLatin1String("data-") + QString::number(uniqueID)).toString();
    hiddenService = new Tor::HiddenService(dir, this);
    connect(hiddenService, SIGNAL(statusChanged(int,int)), SLOT(onStatusChanged(int,int)));

    QHostAddress address(config->value("core/listenIp", QLatin1String("127.0.0.1")).toString());
    quint16 port = (quint16)config->value("core/listenPort", 0).toUInt();

    if (hiddenService->status() == Tor::HiddenService::NotCreated && !readSetting("createNewService", false).toBool())
    {
        qWarning("Hidden service data for identity %d in %s does not exist", uniqueID, qPrintable(dir));
        delete hiddenService;
        hiddenService = 0;
    }
    else
    {
        incomingSocket = new IncomingSocket(this, this);
        if (!incomingSocket->listen(address, port))
        {
            qWarning("Failed to open incoming socket: %s", qPrintable(incomingSocket->errorString()));
            return;
        }

        hiddenService->addTarget(80, incomingSocket->serverAddress(), incomingSocket->serverPort());
        torManager->addHiddenService(hiddenService);
    }

    connect(torManager, SIGNAL(socksReady()), &contacts,  SLOT(connectToAll()));
}

UserIdentity *UserIdentity::createIdentity(int uniqueID, const QString &dataDirectory)
{
    config->beginGroup(QString::fromLatin1("identity/%1").arg(uniqueID));
    config->setValue("createNewService", true);
    if (dataDirectory.isEmpty())
        config->setValue("dataDirectory", QLatin1String("data-") + QString::number(uniqueID));
    else
        config->setValue("dataDirectory", dataDirectory);
    config->endGroup();

    return new UserIdentity(uniqueID);
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

QString UserIdentity::contactID() const
{
    return ContactIDValidator::idFromHostname(hostname());
}

void UserIdentity::setNickname(const QString &nick)
{
    if (nick == m_nickname)
        return;

    m_nickname = nick;
    writeSetting("nickname", nick);
}

QString UserIdentity::avatarCacheKey(AvatarSize size)
{
    return QString::fromLatin1("id-avatar-%1-%2").arg(uniqueID).arg(int(size));
}

QPixmap UserIdentity::avatar(AvatarSize size)
{
    QPixmap re;
    QString cacheKey = avatarCacheKey(size);
    if (QPixmapCache::find(cacheKey, re))
        return re;

    QByteArray data = readSetting((size == TinyAvatar) ? "avatar-tiny" : "avatar").toByteArray();
    if (data.isEmpty())
        return QPixmap();

    re.loadFromData(data);

    QPixmapCache::insert(cacheKey, re);
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
            QPixmapCache::insert(avatarCacheKey(FullAvatar), QPixmap::fromImage(image));

            QImage tiny = image.scaled(QSize(35, 35), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            buffer.close();
            buffer.open(QBuffer::ReadWrite);
            if (tiny.save(&buffer, "jpeg", 100))
            {
                QPixmapCache::insert(avatarCacheKey(TinyAvatar), QPixmap::fromImage(tiny));
                writeSetting("avatar-tiny", buffer.buffer());
            }
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

        QPixmapCache::remove(avatarCacheKey(FullAvatar));
        QPixmapCache::remove(avatarCacheKey(TinyAvatar));
    }
}

void UserIdentity::onStatusChanged(int newStatus, int oldStatus)
{
    if (oldStatus == Tor::HiddenService::NotCreated && newStatus > oldStatus)
        removeSetting("createNewService");
    emit statusChanged();
}

bool UserIdentity::isServiceOnline() const
{
    return hiddenService && hiddenService->status() == Tor::HiddenService::Online;
}

bool UserIdentity::isServicePublished() const
{
    return hiddenService && hiddenService->status() >= Tor::HiddenService::Published;
}
