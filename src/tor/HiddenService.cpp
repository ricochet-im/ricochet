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

#include "HiddenService.h"
#include "TorControlManager.h"
#include "TorServiceTest.h"
#include "utils/CryptoKey.h"
#include <QDir>
#include <QFile>
#include <QTimer>

using namespace Tor;

HiddenService::HiddenService(const QString &p, QObject *parent)
    : QObject(parent), dataPath(p), selfTest(0), pStatus(NotCreated)
{
    /* Set the initial status and, if possible, load the hostname */
    QDir dir(dataPath);
    if (dir.exists(QLatin1String("hostname")) && dir.exists(QLatin1String("private_key")))
    {
        readHostname();
        if (!pHostname.isEmpty())
            pStatus = Offline;
    }
}

void HiddenService::setStatus(Status newStatus)
{
    if (pStatus == newStatus)
        return;

    Status old = pStatus;
    pStatus = newStatus;

    emit statusChanged(pStatus, old);

    if (pStatus == Online)
        emit serviceOnline();
}

void HiddenService::addTarget(const Target &target)
{
    pTargets.append(target);
}

void HiddenService::addTarget(quint16 servicePort, QHostAddress targetAddress, quint16 targetPort)
{
    Target t = { targetAddress, servicePort, targetPort };
    pTargets.append(t);
}

void HiddenService::readHostname()
{
    pHostname.clear();

    QFile file(dataPath + QLatin1String("/hostname"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open hostname file for hidden service" << dataPath << "-" << file.errorString();
        return;
    }

    QByteArray data;
    data.resize(32);

    int rd = file.readLine(data.data(), data.size());
    if (rd < 0)
    {
        qDebug() << "Failed to read hostname file for hidden service" << dataPath << "-" << file.errorString();
        return;
    }

    data.resize(rd);

    int sep = data.lastIndexOf('.');
    if (sep != 16 || data.mid(sep) != ".onion\n")
    {
        qDebug() << "Failed to read hostname file for hidden service" << dataPath << "- invalid contents";
        return;
    }

    pHostname = QString::fromLatin1(data.constData(), sep) + QLatin1String(".onion");
    qDebug() << "Hidden service hostname is" << pHostname;
}

CryptoKey HiddenService::cryptoKey() const
{
    CryptoKey key;

    bool ok = key.loadFromFile(dataPath + QLatin1String("/private_key"), true);
    if (!ok)
        qWarning() << "Failed to load hidden service key";

    return key;
}

void HiddenService::startSelfTest()
{
    if (pHostname.isEmpty() || pTargets.isEmpty())
    {
        if (selfTest)
        {
            delete selfTest;
            selfTest = 0;
        }

        return;
    }

    if (!selfTest)
    {
        selfTest = new TorServiceTest(torManager);
        connect(selfTest, SIGNAL(success()), this, SLOT(selfTestSucceeded()));
        connect(selfTest, SIGNAL(failure()), this, SLOT(selfTestFailed()));
    }

    /* XXX Should probably try all targets */
    selfTest->connectToHost(hostname(), pTargets[0].servicePort);
}

void HiddenService::servicePublished()
{
    readHostname();

    if (pHostname.isEmpty())
    {
        qDebug() << "Failed to read hidden service hostname";
        return;
    }

    qDebug() << "Hidden service published successfully";
    setStatus(Published);

    startSelfTest();
}

void HiddenService::selfTestSucceeded()
{
    qDebug() << "Hidden service self-test completed successfully";
    setStatus(Online);

    selfTest->deleteLater();
    selfTest = 0;
}

void HiddenService::selfTestFailed()
{
    qDebug() << "Hidden service self-test failed; trying again in 10 seconds";
    setStatus(Published);

    QTimer::singleShot(10000, this, SLOT(startSelfTest()));
}
