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
#include "utils/OSUtil.h"
#include "utils/CryptoKey.h"
#include "utils/SecureRNG.h"
#include <QApplication>
#include <QFile>
#include <QTcpServer>
#include <QDebug>
#include <QTimer>

using namespace Tor;

BundledTorManager *BundledTorManager::m_instance = 0;

BundledTorManager::BundledTorManager()
    : m_torControl(0), m_killAttempts(0), controlPort(0), socksPort(0), m_wantExit(false)
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
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(shutdown()));
}

void BundledTorManager::start()
{
    if (isRunning())
        return;

    Q_ASSERT(m_torControl);

    QString dir = torDirectory();

    qint64 oldPid = readPidFile(dir + QLatin1String("pid"));
    if (oldPid > 0 && isProcessRunning(oldPid))
    {
        /* Existing instance is running, but we can't connect to it, because the
         * control port, socks port, and password are all unknown now. Kill the
         * existing process, and start a new one once it has exited */
        if (m_killAttempts && ++m_killAttempts < 10)
        {
            QTimer::singleShot(500, this, SLOT(start()));
            return;
        }

        bool force = (m_killAttempts == 10);
        qDebug() << "Attempting to kill old bundled tor instance" << (force ? "forcefully" : "");
        if (m_killAttempts > 10 || !killProcess(oldPid, force))
        {
            qWarning() << "Failed to kill old instance of bundled tor; trying to start anyway";
        }
        else
        {
            QTimer::singleShot(500, this, SLOT(start()));
            return;
        }
    }

    m_killAttempts = 0;

    QStringList args;

    QString torrc = dir + QLatin1String("torrc");
    if (!QFile::exists(torrc))
    {
        if (!QFile(torrc).open(QIODevice::ReadWrite))
        {
            emit torError(QLatin1String("Cannot create configuration file"));
            return;
        }
    }

    /* Files */
    args << QLatin1String("-f") << QLatin1String("torrc");
    args << QLatin1String("--DataDirectory") << QLatin1String(".");
    args << QLatin1String("--PidFile") << QLatin1String("pid");

    /* Ports */
    if (!selectAvailablePorts())
    {
        emit torError(QLatin1String("Unable to choose local ports"));
        return;
    }

    args << QLatin1String("--SocksPort") << QString::number(socksPort);
    args << QLatin1String("--ControlPort") << QString::number(controlPort);

    /* Password */
    QByteArray password = SecureRNG::random(16);
    QByteArray hashedPassword = torControlHashedPassword(password);
    if (password.isNull() || hashedPassword.isNull())
    {
        emit torError(QLatin1String("Random number generation failed"));
        return;
    }

    m_torControl->setAuthPassword(password);
    args << QLatin1String("--__HashedControlSessionPassword") << QString::fromLatin1(hashedPassword);

    /* Launch it. */
    qDebug() << "Launching bundled Tor from" << torExecutable();

    m_wantExit = false;

    process.setWorkingDirectory(dir);
    process.start(torExecutable(), args, QIODevice::ReadOnly);

    m_torControl->connect(QHostAddress::LocalHost, controlPort);
}

/* Does not ensure that Tor actually exits; it just asks nicely. This is intended for application quit. */
void BundledTorManager::shutdown()
{
    if (m_torControl)
        m_torControl->shutdownSync();
    m_wantExit = true;
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
    if (m_wantExit)
    {
        m_wantExit = false;
        return;
    }

    /* Exit was unexpected */
    qWarning() << "Bundled tor process exited unexpectedly:" << exitCode << process.errorString();
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
