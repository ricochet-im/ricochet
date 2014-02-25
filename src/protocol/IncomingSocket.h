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

#ifndef INCOMINGSOCKET_H
#define INCOMINGSOCKET_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QBasicTimer>
#include "ProtocolConstants.h"

class QTcpServer;
class QTcpSocket;
class UserIdentity;

class IncomingSocket : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IncomingSocket)

public:
    UserIdentity * const identity;

    explicit IncomingSocket(UserIdentity *identity, QObject *parent = 0);

    bool listen(const QHostAddress &address, quint16 port);
    QString errorString() const;

    QHostAddress serverAddress() const;
    quint16 serverPort() const;

    /* The intro based on supported protocols; anything after the purpose is left to the caller. */
    static QByteArray introData(Protocol::Purpose purpose);

private slots:
    void incomingConnection();

    void readSocket();
    void removeSocket(QTcpSocket *socket = 0);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QTcpServer *server;
    QList<QTcpSocket*> pendingSockets;
    QBasicTimer expireTimer;

    bool handleVersion(QTcpSocket *socket);
    void handleIntro(QTcpSocket *socket, uchar version);
};

#endif // INCOMINGSOCKET_H
