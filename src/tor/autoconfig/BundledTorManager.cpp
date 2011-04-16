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

#include "BundledTorManager.h"
#include "tor/TorControlManager.h"
#include "utils/AppSettings.h"
#include "utils/OSUtil.h"
#include "utils/CryptoKey.h"
#include "utils/SecureRNG.h"
#include <QApplication>
#include <QFile>
#include <QDir>
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

static QString torDataDirectory()
{
    return config->configLocation() + QLatin1String("tor/");
}

static QString torExecutable()
{
    static QString dir;
    if (dir.isNull())
    {
        dir = qApp->applicationDirPath() + QLatin1String("/tor/");
#ifdef BUNDLED_TOR_PATH
        if (!QFile::exists(dir))
            dir = QString::fromLatin1(BUNDLED_TOR_PATH);
#endif
    }

#ifdef Q_OS_WIN
    return dir + QLatin1String("tor.exe");
#else
    return dir + QLatin1String("tor");
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

    QString dir = torDataDirectory();

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
        if (!QFile::exists(dir))
        {
            QDir pd(dir);
            QString dirname = pd.dirName();
            if (!pd.cdUp() || !pd.mkdir(dirname))
            {
                emit torError(QLatin1String("Cannot create data directory"));
                return;
            }
        }

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
    Q_UNUSED(status);

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
