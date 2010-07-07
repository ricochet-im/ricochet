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

#include "TorServiceTest.h"
#include "TorControlManager.h"
#include <QNetworkProxy>

using namespace Tor;

TorServiceTest::TorServiceTest(TorControlManager *m)
    : QObject(m), manager(m), socket(new QTcpSocket(this)), state(-1)
{
    connect(manager, SIGNAL(socksReady()), this, SLOT(socksReady()));

    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(socketError(QAbstractSocket::SocketError)));
}

void TorServiceTest::connectToHost(const QString &host, quint16 port)
{
    if (socket->state() != QAbstractSocket::UnconnectedState)
        socket->abort();

    this->host = host;
    this->port = port;
    state = -1;

    if (!manager->isSocksReady())
    {
        qDebug() << "Tor self-test waiting for SOCKS to be ready";
        return;
    }

    socket->setProxy(manager->connectionProxy());
    socket->connectToHost(host, port);
}

void TorServiceTest::socketConnected()
{
    state = 1;
    emit finished(true);
    emit success();

    socket->close();
}

void TorServiceTest::socketError(QAbstractSocket::SocketError)
{
    state = 0;
    emit finished(false);
    emit failure();
}

void TorServiceTest::socksReady()
{
    if (state >= 0 || host.isEmpty())
        return;

    connectToHost(host, port);
}
