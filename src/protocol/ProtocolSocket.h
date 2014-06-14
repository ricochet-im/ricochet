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

#ifndef PROTOCOLSOCKET_H
#define PROTOCOLSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <QHash>
#include <QElapsedTimer>

class ProtocolCommand;
class ContactUser;

/* Send and receive commands with a contact over an established and
 * authenticated socket. The socket may be established locally or remotely.
 * There is generally one instance per contact, and the socket may be
 * lost or replaced at any time. */
class ProtocolSocket : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProtocolSocket)

public:
    ContactUser * const user;
    explicit ProtocolSocket(ContactUser *user);

    /* Connected and authenticated network socket.
     * Ownership of the socket is taken and it will be deleted if replaced.
     */
    QTcpSocket *socket() { return m_socket; }
    void setSocket(QTcpSocket *socket);

    bool isConnected() const;
    int connectedDuration() const;

    /* Get an available identifier; not reserved, must be followed by sendCommand immediately. */
    quint16 getIdentifier();

    void sendCommand(ProtocolCommand *command);

signals:
    /* Moved to a connected state from a disconnected state */
    void connected();
    /* Moved to a disconnected state from a connected state */
    void disconnected();
    /* Socket has changed. This may happen without connected() when replacing. */
    void socketChanged();

public slots:
    void disconnect();

private slots:
    void abortCommands();
    void flushCommands();
    void read();
    void socketDisconnected();

private:
    QQueue<ProtocolCommand*> commandQueue;
    QHash<quint16,ProtocolCommand*> pendingCommands;
    QTcpSocket *m_socket;
    QElapsedTimer m_connectedTime;
    quint16 nextCommandId;
};

#endif // PROTOCOLSOCKET_H
