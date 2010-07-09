/* Torsion - http://github.com/special/torsion
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

#include "BundledTorManager.h"
#include "tor/TorControlManager.h"
#include <QApplication>
#include <QFile>
#include <QTcpServer>
#include <QDebug>

using namespace Tor;

BundledTorManager *BundledTorManager::m_instance = 0;

BundledTorManager::BundledTorManager()
    : m_torControl(0), controlPort(0), socksPort(0)
{
    Q_ASSERT(!m_instance);
    m_instance = this;

    process.setProcessChannelMode(QProcess::MergedChannels);
    process.setReadChannel(QProcess::StandardOutput);
    connect(&process, SIGNAL(readyRead()), SLOT(readOutput()));
    connect(&process, SIGNAL(finished(int,QProcess::ExitStatus)), SLOT(processExited(int,QProcess::ExitStatus)));
}

BundledTorManager *BundledTorManager::instance()
{
    if (!m_instance)
        m_instance = new BundledTorManager;

    return m_instance;
}

static QString torDirectory()
{
    return qApp->applicationDirPath() + QLatin1String("/Tor/");
}

static QString torExecutable()
{
#ifdef Q_OS_WIN
    return torDirectory() + QLatin1String("tor.exe");
#else
    return torDirectory() + QLatin1String("tor");
#endif
}

bool BundledTorManager::isAvailable()
{
    return QFile::exists(torExecutable());
}

void BundledTorManager::setTorManager(Tor::TorControlManager *tor)
{
    m_torControl = tor;
}

void BundledTorManager::start()
{
    if (isRunning())
        return;

    Q_ASSERT(m_torControl);

    qDebug() << "Launching bundled Tor from" << torExecutable();

    QString dir = torDirectory();
    QStringList args;

    QString torrc = dir + QLatin1String("torrc");
    if (!QFile::exists(torrc))
    {
        if (!QFile(torrc).open(QIODevice::ReadWrite))
        {
            /* TODO handle errors */
            qWarning() << "Failed to create torrc for bundled tor";
            return;
        }
    }

    args << QLatin1String("-f") << QLatin1String("torrc");
    args << QLatin1String("--DataDirectory") << QLatin1String(".");

    if (!selectAvailablePorts())
    {
        qWarning() << "Failed to select ports for bundled tor";
        return;
    }

    args << QLatin1String("--SocksPort") << QString::number(socksPort);
    args << QLatin1String("--ControlPort") << QString::number(controlPort);
    // HashedControlPassword

    process.setWorkingDirectory(dir);
    process.start(torExecutable(), args, QIODevice::ReadOnly);

    m_torControl->connect(QHostAddress::LocalHost, controlPort);
}

void BundledTorManager::readOutput()
{
    for (;;)
    {
        QByteArray line = process.readLine(1024);
        if (line.isEmpty())
            return;

        qDebug() << "Tor:" << line.trimmed();
    }
}

void BundledTorManager::processExited(int exitCode, QProcess::ExitStatus status)
{
    qDebug() << "Tor exited:" << exitCode << status << process.error() << process.errorString();
}

bool BundledTorManager::selectAvailablePorts()
{
    /* Select two different random ports that are not in use, for SOCKS and control */
    controlPort = socksPort = 0;

    QTcpServer testSocket;
    for (;;)
    {
        quint16 port = quint16((qrand() % 45535) + 20000);
        if (!testSocket.listen(QHostAddress::LocalHost, port))
        {
            if (testSocket.serverError() == QAbstractSocket::AddressInUseError)
                continue;
            return false;
        }

        testSocket.close();

        if (!controlPort)
        {
            controlPort = port;
        }
        else if (controlPort != port)
        {
            socksPort = port;
            break;
        }
    }

    return true;
}
