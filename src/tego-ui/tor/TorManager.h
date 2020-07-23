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

#ifndef TORMANAGER_H
#define TORMANAGER_H

#include <QObject>
#include <QStringList>

namespace Tor
{

class TorProcess;
class TorControl;
class TorManagerPrivate;

/* Run/connect to an instance of Tor according to configuration, and manage
 * UI interaction, first time configuration, etc. */
class TorManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool configurationNeeded READ configurationNeeded NOTIFY configurationNeededChanged)
    Q_PROPERTY(QStringList logMessages READ logMessages CONSTANT)
    Q_PROPERTY(Tor::TorProcess* process READ process CONSTANT)
    Q_PROPERTY(Tor::TorControl* control READ control CONSTANT)
    Q_PROPERTY(bool hasError READ hasError NOTIFY errorChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
    Q_PROPERTY(QString dataDirectory READ dataDirectory WRITE setDataDirectory)

public:
    explicit TorManager(QObject *parent = 0);
    static TorManager *instance();

    TorProcess *process();
    TorControl *control();

    QString dataDirectory() const;
    void setDataDirectory(const QString &path);

    // True on first run or when the Tor configuration wizard needs to be shown
    bool configurationNeeded() const;

    QStringList logMessages() const;

    bool hasError() const;
    QString errorMessage() const;

public slots:
    void start();

signals:
    void configurationNeededChanged();
    void errorChanged();

private:
    TorManagerPrivate *d;
};

}

#endif
#
