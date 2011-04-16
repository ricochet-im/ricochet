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

#ifndef PROTOCOLSOCKET_H
#define PROTOCOLSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <QHash>

class ProtocolManager;
class ProtocolCommand;

class ProtocolSocket : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProtocolSocket)

public:
    enum Purpose
    {
        PurposePrimary = 0x00,
        PurposeAuxGeneral = 0x01,
        PurposeContactReq = 0x80,

        PurposeAuxMin = 0x01,
        PurposeAuxMax = 0x20
    };

    ProtocolManager * const manager;
    QTcpSocket * const socket;

    /* Create with an established and authenticated socket (incoming connections) */
    explicit ProtocolSocket(QTcpSocket *socket, ProtocolManager *manager);
    /* Create with a new socket */
    explicit ProtocolSocket(ProtocolManager *manager);

    /* Returns true if the socket is connected and ready (i.e. authenticated) */
    bool isConnected() const;
    bool isConnecting() const;

    void connectToHost(const QString &host, quint16 port);

    /* Get an available identifier; not reserved, must be followed by sendCommand immediately. */
    quint16 getIdentifier();

    void sendCommand(ProtocolCommand *command);

signals:
    /* Connected and authenticated */
    void socketReady();
    /* Disconnected from an authenticated connection */
    void disconnected();
    /* Connection attempt failed or disconnected from an unauthenticated connection */
    void connectFailed();

public slots:
    void abort();
    void abortConnectionAttempt();

private slots:
    void sendAuth();
    void flushCommands();

    void read();
    void socketDisconnected();

private:
    QQueue<ProtocolCommand*> commandQueue;
    QHash<quint16,ProtocolCommand*> pendingCommands;
    quint16 nextCommandId;

    bool active, authPending, authFinished;

    void setupSocket();
};

#endif // PROTOCOLSOCKET_H
