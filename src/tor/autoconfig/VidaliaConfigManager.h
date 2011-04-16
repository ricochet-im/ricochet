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

#ifndef VIDALIACONFIGMANAGER_H
#define VIDALIACONFIGMANAGER_H

#include <QObject>

class QSettings;

class VidaliaConfigManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VidaliaConfigManager)

public:
    explicit VidaliaConfigManager(QObject *parent = 0);

    static bool isVidaliaInstalled();
    static QString vidaliaConfigPath();

    QString path() const { return m_path; }

    qint64 currentPid() const;
    bool isVidaliaRunning() const;

    /* True if Vidalia is setup in a way that we can use without reconfiguration */
    bool hasCompatibleConfig() const;
    /* Alter Vidalia's configuration to make it compatible */
    bool reconfigureControlConfig(QString *errorMessage = 0);

    void getControlInfo(QString *address, quint16 *port, QByteArray *password = 0) const;

private:
    QString m_path;

    QString configPath() const { return path() + QLatin1String("/vidalia.conf"); }
};

#endif // VIDALIACONFIGMANAGER_H
