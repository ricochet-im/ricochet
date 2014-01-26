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

#include "TorServiceTest.h"
#include "TorControl.h"
#include <QNetworkProxy>
#include <QDebug>

using namespace Tor;

TorServiceTest::TorServiceTest(TorControl *m)
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
