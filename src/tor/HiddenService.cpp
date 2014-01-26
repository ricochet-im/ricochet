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

#include "HiddenService.h"
#include "TorControl.h"
#include "TorServiceTest.h"
#include "utils/CryptoKey.h"
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QDebug>

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
