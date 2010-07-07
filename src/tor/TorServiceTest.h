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

#ifndef TORSERVICETEST_H
#define TORSERVICETEST_H

#include <QObject>
#include <QTcpSocket>

namespace Tor
{

class TorControlManager;

class TorServiceTest : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TorServiceTest)

public:
    TorControlManager * const manager;

    explicit TorServiceTest(TorControlManager *manager);

    void connectToHost(const QString &host, quint16 port);

    bool isSuccessful() const { return state == 1; }
    bool isFinished() const { return state >= 0; }

signals:
    void success();
    void failure();
    void finished(bool success);

private slots:
    void socketConnected();
    void socketError(QAbstractSocket::SocketError error);

    void socksReady();

private:
    QTcpSocket *socket;
    QString host;
    quint16 port;
    int state;
};

}

#endif // TORSERVICETEST_H
