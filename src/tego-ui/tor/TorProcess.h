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

#ifndef TORPROCESS_H
#define TORPROCESS_H

#include <QObject>
#include <QHostAddress>

namespace Tor
{

class TorProcessPrivate;

/* Launches and controls a Tor instance with behavior suitable for bundling
 * an instance with the application. */
class TorProcess : public QObject
{
    Q_OBJECT
    Q_ENUMS(State)

    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    enum State {
        Failed = -1,
        NotStarted,
        Starting,
        Connecting,
        Ready
    };

    explicit TorProcess(QObject *parent = 0);
    virtual ~TorProcess();

    QString executable() const;
    void setExecutable(const QString &path);

    QString dataDir() const;
    void setDataDir(const QString &path);

    QString defaultTorrc() const;
    void setDefaultTorrc(const QString &path);

    QStringList extraSettings() const;
    void setExtraSettings(const QStringList &settings);

    State state() const;
    QString errorMessage() const;
    QHostAddress controlHost();
    quint16 controlPort();
    QByteArray controlPassword();

public slots:
    void start();
    void stop();

signals:
    void stateChanged(int newState);
    void errorMessageChanged(const QString &errorMessage);
    void logMessage(const QString &message);

private:
    TorProcessPrivate *d;
};

}

#endif

