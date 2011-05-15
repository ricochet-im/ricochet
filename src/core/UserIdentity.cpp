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
    emit nicknameChanged();
}

void UserIdentity::setAvatar(QImage image)
{
    if (image.width() > 160 || image.height() > 160)
        image = image.scaled(QSize(160, 160), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (!image.isNull())
    {
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        if (image.save(&buffer, "jpeg", 90))
            writeSetting("avatar", buffer.buffer());
        else
            image = QImage();
    }

    if (image.isNull())
        removeSetting("avatar");
}

void UserIdentity::onStatusChanged(int newStatus, int oldStatus)
{
    if (oldStatus == Tor::HiddenService::NotCreated && newStatus > oldStatus)
    {
        removeSetting("createNewService");
        emit contactIDChanged();
    }
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
