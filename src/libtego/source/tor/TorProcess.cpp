/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
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

#include "TorProcess_p.h"
#include "utils/CryptoKey.h"
#include "utils/SecureRNG.h"

using namespace Tor;

TorProcess::TorProcess(QObject *parent)
    : QObject(parent), d(new TorProcessPrivate(this))
{
}

TorProcess::~TorProcess()
{
    if (state() > NotStarted)
        stop();
}

TorProcessPrivate::TorProcessPrivate(TorProcess *tp)
    : QObject(tp), q(tp), state(TorProcess::NotStarted), controlPort(0), controlPortAttempts(0)
{
    connect(&process, &QProcess::started, this, &TorProcessPrivate::processStarted);
    //XXX: This static cast shouldn't be needed here, but it is. Why?
    connect(&process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &TorProcessPrivate::processFinished);
    connect(&process, &QProcess::errorOccurred, this, &TorProcessPrivate::processError);
    connect(&process, &QProcess::readyRead, this, &TorProcessPrivate::processReadable);

    controlPortTimer.setInterval(500);
    connect(&controlPortTimer, &QTimer::timeout, this, &TorProcessPrivate::tryReadControlPort);
}

QString TorProcess::executable() const
{
    return d->executable;
}

void TorProcess::setExecutable(const QString &path)
{
    d->executable = path;
}

QString TorProcess::dataDir() const
{
    return d->dataDir;
}

void TorProcess::setDataDir(const QString &path)
{
    d->dataDir = path;
}

QString TorProcess::defaultTorrc() const
{
    return d->defaultTorrc;
}

void TorProcess::setDefaultTorrc(const QString &path)
{
    d->defaultTorrc = path;
}

QStringList TorProcess::extraSettings() const
{
    return d->extraSettings;
}

void TorProcess::setExtraSettings(const QStringList &settings)
{
    d->extraSettings = settings;
}

TorProcess::State TorProcess::state() const
{
    return d->state;
}

QString TorProcess::errorMessage() const
{
    return d->errorMessage;
}

void TorProcess::start()
{
    if (state() > NotStarted)
        return;

    d->errorMessage.clear();

    if (d->executable.isEmpty() || d->dataDir.isEmpty()) {
        d->errorMessage = QStringLiteral("Tor executable and data directory not specified");
        d->state = Failed;
        emit errorMessageChanged(d->errorMessage);
        emit stateChanged(d->state);
        return;
    }

    if (!d->ensureFilesExist()) {
        d->state = Failed;
        emit errorMessageChanged(d->errorMessage);
        emit stateChanged(d->state);
        return;
    }

    QByteArray password = controlPassword();
    QByteArray hashedPassword = torControlHashedPassword(password);
    if (password.isEmpty() || hashedPassword.isEmpty()) {
        d->errorMessage = QStringLiteral("Random password generation failed");
        d->state = Failed;
        emit errorMessageChanged(d->errorMessage);
        emit stateChanged(d->state);
    }

    QStringList args;
    if (!d->defaultTorrc.isEmpty())
        args << QStringLiteral("--defaults-torrc") << d->defaultTorrc;
    args << QStringLiteral("-f") << d->torrcPath();
    args << QStringLiteral("DataDirectory") << d->dataDir;
    args << QStringLiteral("HashedControlPassword") << QString::fromLatin1(hashedPassword);
    args << QStringLiteral("ControlPort") << QStringLiteral("auto");
    args << QStringLiteral("ControlPortWriteToFile") << d->controlPortFilePath();
    args << QStringLiteral("__OwningControllerProcess") << QString::number(qApp->applicationPid());
    args << d->extraSettings;

    d->state = Starting;
    emit stateChanged(d->state);

    if (QFile::exists(d->controlPortFilePath()))
        QFile::remove(d->controlPortFilePath());
    d->controlPort = 0;
    d->controlHost.clear();

    d->process.setProcessChannelMode(QProcess::MergedChannels);
    d->process.start(d->executable, args, QIODevice::ReadOnly);
}

void TorProcess::stop()
{
    if (state() < Starting)
        return;

    d->controlPortTimer.stop();

    if (d->process.state() == QProcess::Starting)
        d->process.waitForStarted(2000);

    d->state = NotStarted;

    // Windows can't terminate the process well, but Tor will clean itself up
#ifndef Q_OS_WIN
    if (d->process.state() == QProcess::Running) {
        d->process.terminate();
        if (!d->process.waitForFinished(5000)) {
            qWarning() << "Tor process" << d->process.pid() << "did not respond to terminate, killing...";
            d->process.kill();
            if (!d->process.waitForFinished(2000)) {
                qCritical() << "Tor process" << d->process.pid() << "did not respond to kill!";
            }
        }
    }
#endif

    emit stateChanged(d->state);
}

QByteArray TorProcess::controlPassword()
{
    if (d->controlPassword.isEmpty())
        d->controlPassword = SecureRNG::randomPrintable(16);
    return d->controlPassword;
}

QHostAddress TorProcess::controlHost()
{
    return d->controlHost;
}

quint16 TorProcess::controlPort()
{
    return d->controlPort;
}

bool TorProcessPrivate::ensureFilesExist()
{
    QFile torrc(torrcPath());
    if (!torrc.exists()) {
        QDir dir(dataDir);
        if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
            errorMessage = QStringLiteral("Cannot create Tor data directory: %1").arg(dataDir);
            return false;
        }

        if (!torrc.open(QIODevice::ReadWrite)) {
            errorMessage = QStringLiteral("Cannot create Tor configuration file: %1").arg(torrcPath());
            return false;
        }
    }

    return true;
}

QString TorProcessPrivate::torrcPath() const
{
    return QDir::toNativeSeparators(dataDir) + QDir::separator() + QStringLiteral("torrc");
}

QString TorProcessPrivate::controlPortFilePath() const
{
    return QDir::toNativeSeparators(dataDir) + QDir::separator() + QStringLiteral("control-port");
}

void TorProcessPrivate::processStarted()
{
    state = TorProcess::Connecting;
    emit q->stateChanged(state);

    controlPortAttempts = 0;
    controlPortTimer.start();
}

void TorProcessPrivate::processFinished()
{
    if (state < TorProcess::Starting)
        return;

    controlPortTimer.stop();
    errorMessage = process.errorString();
    if (errorMessage.isEmpty())
        errorMessage = QStringLiteral("Process exited unexpectedly (code %1)").arg(process.exitCode());
    state = TorProcess::Failed;
    emit q->errorMessageChanged(errorMessage);
    emit q->stateChanged(state);
}

void TorProcessPrivate::processError(QProcess::ProcessError error)
{
    if (error == QProcess::FailedToStart || error == QProcess::Crashed)
        processFinished();
}

void TorProcessPrivate::processReadable()
{
    while (process.bytesAvailable() > 0) {
        QByteArray line = process.readLine(2048).trimmed();
        if (!line.isEmpty())
            emit q->logMessage(QString::fromLatin1(line));
    }
}

void TorProcessPrivate::tryReadControlPort()
{
    QFile file(controlPortFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readLine().trimmed();

        int p;
        if (data.startsWith("PORT=") && (p = data.lastIndexOf(':')) > 0) {
            controlHost = QHostAddress(QString::fromLatin1(data.mid(5, p - 5)));
            controlPort = data.mid(p+1).toUShort();

            if (!controlHost.isNull() && controlPort > 0) {
                controlPortTimer.stop();
                state = TorProcess::Ready;
                emit q->stateChanged(state);
                return;
            }
        }
    }

    if (++controlPortAttempts * controlPortTimer.interval() > 10000) {
        errorMessage = QStringLiteral("No control port available after launching process");
        state = TorProcess::Failed;
        emit q->errorMessageChanged(errorMessage);
        emit q->stateChanged(state);
    }
}

