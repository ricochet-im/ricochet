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

#ifndef PROTOCOL_CONNECTION_P_H
#define PROTOCOL_CONNECTION_P_H

#include "Connection.h"

namespace Protocol
{

class ConnectionPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ConnectionPrivate)

public:
    static const quint8 ProtocolVersion = 1;
    static const quint8 ProtocolVersionFailed = 0xff;
    static const int PacketHeaderSize = 4;
    static const int PacketMaxDataSize = UINT16_MAX - PacketHeaderSize;
    // Time in seconds before a connection with a purpose of Unknown is killed
    static const int UnknownPurposeTimeout = 15;

    explicit ConnectionPrivate(Connection *q);
    virtual ~ConnectionPrivate();

    Connection *q;
    QTcpSocket *socket;
    QHash<int,Channel*> channels;
    QMap<Connection::AuthenticationType,QString> authentication;
    QElapsedTimer ageTimer;
    Connection::Direction direction;
    Connection::Purpose purpose;
    bool wasClosed;
    bool handshakeDone;

    void setSocket(QTcpSocket *socket, Connection::Direction direction);

    int availableOutboundChannelId();
    bool isValidAvailableChannelId(int channelId, Connection::Direction idDirection);

    bool insertChannel(Channel *channel);
    void removeChannel(Channel *channel);

    void closeAllChannels();

    bool writePacket(Channel *channel, const QByteArray &data);
    bool writePacket(int channelId, const QByteArray &data);

public slots:
    void closeImmediately();

private slots:
    void socketReadable();
    void socketDisconnected();

private:
    int nextOutboundChannelId;
};

}

#endif
