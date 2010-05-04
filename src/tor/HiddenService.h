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

#ifndef HIDDENSERVICE_H
#define HIDDENSERVICE_H

#include <QObject>
#include <QHostAddress>
#include <QList>

class CryptoKey;

namespace Tor
{

class HiddenService : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HiddenService)

    friend class TorControlManager;

public:
    struct Target
    {
        QHostAddress targetAddress;
        quint16 servicePort, targetPort;
    };

    enum Status
    {
        Offline, /* Not published */
        Published, /* Published, but not confirmed to be accessible */
        Online /* Published and accessible */
    };

    const QString dataPath;

    HiddenService(const QString &dataPath);

    Status status() const { return pStatus; }

    const QString &hostname() const { return pHostname; }
    CryptoKey cryptoKey() const;

    const QList<Target> &targets() const { return pTargets; }
    void addTarget(const Target &target);
    void addTarget(quint16 servicePort, QHostAddress targetAddress, quint16 targetPort);

public slots:
    void startSelfTest();

signals:
    void statusChanged(int newStatus, int oldStatus);
    void serviceOnline();

private slots:
    void servicePublished();
    void selfTestSucceeded();
    void selfTestFailed();

private:
    QList<Target> pTargets;
    QString pHostname;
    class TorServiceTest *selfTest;
    Status pStatus;

    void setStatus(Status newStatus);
    void readHostname();
};

}

#endif // HIDDENSERVICE_H
