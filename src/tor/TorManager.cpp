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

#include "TorManager.h"
#include "TorProcess.h"
#include "TorControlManager.h"
#include "utils/AppSettings.h"
#include <QFile>
#include <QCoreApplication>

using namespace Tor;

namespace Tor
{

class TorManagerPrivate : public QObject
{
    Q_OBJECT

public:
    TorManager *q;
    TorProcess *process;
    TorControlManager *control;

    explicit TorManagerPrivate(TorManager *parent = 0);

    QString torExecutablePath() const;

public slots:
    void processStateChanged(int state);
    void processErrorChanged(const QString &errorMessage);
    void processLogMessage(const QString &message);
};

}

TorManager::TorManager(QObject *parent)
    : QObject(parent), d(new TorManagerPrivate(this))
{
}

TorManagerPrivate::TorManagerPrivate(TorManager *parent)
    : QObject(parent)
    , q(parent)
    , process(0)
    , control(new TorControlManager(this))
{
}

TorControlManager *TorManager::control()
{
    return d->control;
}

TorProcess *TorManager::process()
{
    return d->process;
}

void TorManager::start()
{
    if (config->value("tor/controlPort").isNull()) {
        // Launch a bundled Tor instance
        QString executable = d->torExecutablePath();
        if (executable.isEmpty()) {
            // XXX error
            qFatal("Implement your error conditions.");
        }

        if (!d->process) {
            d->process = new TorProcess(this);
            connect(d->process, SIGNAL(stateChanged(int)), d, SLOT(processStateChanged(int)));
            connect(d->process, SIGNAL(errorMessageChanged(QString)), d,
                    SLOT(processErrorChanged(QString)));
            connect(d->process, SIGNAL(logMessage(QString)), d, SLOT(processLogMessage(QString)));
        }

        d->process->setExecutable(executable);
        d->process->setDataDir(config->configLocation() + QStringLiteral("tor/"));
        d->process->start();
    } else {
        QHostAddress address(config->value("tor/controlIp", QStringLiteral("127.0.0.1")).toString());
        quint16 port = (quint16)config->value("tor/controlPort", 9051).toUInt();

        d->control->setAuthPassword(config->value("tor/authPassword").toByteArray());
        d->control->connect(address, port);
    }
}

void TorManagerPrivate::processStateChanged(int state)
{
    qDebug() << Q_FUNC_INFO << state << TorProcess::Ready << process->controlPassword() << process->controlHost() << process->controlPort();
    if (state == TorProcess::Ready) {
        control->setAuthPassword(process->controlPassword());
        control->connect(process->controlHost(), process->controlPort());
    }
}

void TorManagerPrivate::processErrorChanged(const QString &errorMessage)
{
    qDebug() << "tor error:" << errorMessage;
}

void TorManagerPrivate::processLogMessage(const QString &message)
{
    qDebug() << "tor:" << message;
}

QString TorManagerPrivate::torExecutablePath() const
{
    QString path = config->value("tor/executablePath").toString();
    if (!path.isEmpty())
        return path;

#ifdef Q_OS_WIN
    QString filename(QStringLiteral("/tor.exe"));
#else
    QString filename(QStringLiteral("/tor"));
#endif

    path = qApp->applicationDirPath();
    if (QFile::exists(path + filename))
        return path + filename;

#ifdef BUNDLED_TOR_PATH
    path = QStringLiteral(BUNDLED_TOR_PATH);
    if (QFile::exists(path + filename))
        return path + filename;
#endif

    return QString();
}

#include "TorManager.moc"

