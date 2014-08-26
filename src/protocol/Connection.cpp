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

#include "Connection_p.h"
#include "utils/Useful.h"
#include <QTcpSocket>
#include <QtEndian>
#include <QDebug>

using namespace Protocol;

Connection::Connection(QTcpSocket *socket, Direction direction, QObject *parent)
    : QObject(parent)
    , d(new ConnectionPrivate(this))
{
    d->setSocket(socket, direction);
}

ConnectionPrivate::ConnectionPrivate(Connection *qq)
    : QObject(qq)
    , q(qq)
    , socket(0)
    , direction(Connection::ClientSide)
    , nextOutboundChannelId(-1)
{
    ageTimer.start();
}

Connection::Direction Connection::direction() const
{
    return d->direction;
}

bool Connection::isConnected() const
{
    return d->socket && d->socket->state() == QAbstractSocket::ConnectedState;
}

QString Connection::serverHostname() const
{
    QString hostname;
    if (direction() == ClientSide)
        hostname = d->socket->peerName();
    else if (direction() == ServerSide)
        hostname = d->socket->property("localHostname").toString();

    if (!hostname.endsWith(QStringLiteral(".onion"))) {
        BUG() << "Connection does not have a valid server hostname:" << hostname;
        return QString();
    }

    return hostname;
}

int Connection::age() const
{
    return qRound(d->ageTimer.elapsed() / 1000.0);
}

void ConnectionPrivate::setSocket(QTcpSocket *s, Connection::Direction d)
{
    if (socket) {
        BUG() << "Connection already has a socket";
        return;
    }

    socket = s;
    direction = d;
    connect(socket, &QAbstractSocket::disconnected, this, &ConnectionPrivate::socketDisconnected);
    connect(socket, &QIODevice::readyRead, this, &ConnectionPrivate::socketReadable);

    socket->setParent(q);

    if (socket->state() != QAbstractSocket::ConnectedState) {
        BUG() << "Connection created with socket in a non-connected state" << socket->state();
    }

    if (direction == Connection::ClientSide) {
        // The server side is implicitly authenticated (by the transport) as the correct service, so grant that
        QString serverName = q->serverHostname();
        if (serverName.isEmpty()) {
            BUG() << "Server side of connection doesn't have an authenticated name, aborting";
            q->close();
            return;
        }

        q->grantAuthentication(Connection::HiddenServiceAuth, serverName);
    }
}

void Connection::close()
{
    // abort() will discard the contents of the write buffer and close immediately
    // XXX this might not be good, e.g. for "auth rejected, go away" messages.
    if (d->socket)
        d->socket->abort();
}

void ConnectionPrivate::socketDisconnected()
{
    closeAllChannels();
    emit q->closed();
}

void ConnectionPrivate::socketReadable()
{
    qint64 available;
    while ((available = socket->bytesAvailable()) >= PacketHeaderSize) {
        uchar header[PacketHeaderSize];
        // Peek at the header first, to read the size of the packet and make sure
        // the entire thing is available within the buffer.
        qint64 re = socket->peek(reinterpret_cast<char*>(header), PacketHeaderSize);
        if (re < 0) {
            qDebug() << "Connection socket error" << socket->error() << "during read:" << socket->errorString();
            socket->abort();
            return;
        } else if (re < PacketHeaderSize) {
            BUG() << "Socket had" << available << "bytes available but peek only returned" << re;
            return;
        }

        Q_STATIC_ASSERT(PacketHeaderSize == 4);
        quint16 packetSize = qFromBigEndian<quint16>(header);
        quint16 channelId = qFromBigEndian<quint16>(&header[2]);

        if (packetSize < PacketHeaderSize) {
            qWarning() << "Corrupted data from connection (packet size is too small); disconnecting";
            socket->abort();
            return;
        }

        if (packetSize > available)
            break;

        // Read header out of the buffer and discard
        re = socket->read(reinterpret_cast<char*>(header), PacketHeaderSize);
        if (re != PacketHeaderSize) {
            if (re < 0) {
                qDebug() << "Connection socket error" << socket->error() << "during read:" << socket->errorString();
            } else {
                // Because of QTcpSocket buffering, we can expect that up to 'available' bytes
                // will read. Treat anything less as an error condition.
                BUG() << "Socket read was unexpectedly small;" << available << "bytes should've been available but we read" << re;
            }
            socket->abort();
            return;
        }

        // Read data
        QByteArray data(packetSize - PacketHeaderSize, 0);
        re = (data.size() == 0) ? 0 : socket->read(data.data(), data.size());
        if (re != data.size()) {
            if (re < 0) {
                qDebug() << "Connection socket error" << socket->error() << "during read:" << socket->errorString();
            } else {
                // As above
                BUG() << "Socket read was unexpectedly small;" << available << "bytes should've been available but we read" << re;
            }
            socket->abort();
            return;
        }

        Channel *channel = q->channel(channelId);
        if (!channel) {
            // XXX We should sanity-check and rate limit these responses better
            if (data.isEmpty()) {
                qDebug() << "Ignoring channel close message for non-existent channel" << channelId;
            } else {
                qDebug() << "Ignoring" << data.size() << "byte packet for non-existent channel" << channelId;
                // Send channel close message
                writePacket(channelId, QByteArray());
            }
            continue;
        }

        if (channel->connection() != q) {
            // If this fails, something is extremely broken. It may be dangerous to continue
            // processing any data at all. Crash gracefully.
            BUG() << "Channel" << channelId << "found on connection" << this << "but its connection is"
                  << channel->connection();
            qFatal("Connection mismatch while handling packet");
            return;
        }

        if (data.isEmpty()) {
            channel->closeChannel();
        } else {
            channel->receivePacket(data);
        }
    }
}

bool ConnectionPrivate::writePacket(Channel *channel, const QByteArray &data)
{
    if (channel->connection() != q) {
        // As above, dangerously broken, crash the process to avoid damage
        BUG() << "Writing packet for channel" << channel->identifier() << "on connection" << this
              << "but its connection is" << channel->connection();
        qFatal("Connection mismatch while writing packet");
        return false;
    }

    return writePacket(channel->identifier(), data);
}

bool ConnectionPrivate::writePacket(int channelId, const QByteArray &data)
{
    if (channelId < 0 || channelId > UINT16_MAX) {
        BUG() << "Cannot write packet for channel with invalid identifier" << channelId;
        return false;
    }

    if (data.size() > PacketMaxDataSize) {
        BUG() << "Cannot write oversized packet of" << data.size() << "bytes to channel" << channelId;
        return false;
    }

    if (!q->isConnected()) {
        qDebug() << "Cannot write packet to closed connection";
        return false;
    }

    Q_STATIC_ASSERT(PacketHeaderSize + PacketMaxDataSize <= UINT16_MAX);
    Q_STATIC_ASSERT(PacketHeaderSize == 4);
    uchar header[PacketHeaderSize] = { 0 };
    qToBigEndian(static_cast<quint16>(PacketHeaderSize + data.size()), header);
    qToBigEndian(static_cast<quint16>(channelId), &header[2]);

    qint64 re = socket->write(reinterpret_cast<char*>(header), PacketHeaderSize);
    if (re != PacketHeaderSize) {
        qDebug() << "Connection socket error" << socket->error() << "during write:" << socket->errorString();
        socket->abort();
        return false;
    }

    re = socket->write(data);
    if (re != data.size()) {
        qDebug() << "Connection socket error" << socket->error() << "during write:" << socket->errorString();
        socket->abort();
        return false;
    }

    return true;
}

int ConnectionPrivate::availableOutboundChannelId()
{
    int minId, maxId;
    if (direction == Connection::ClientSide) {
        minId = 1;
        maxId = 0x7fff;
    } else {
        minId = 0x8000;
        maxId = 0xffff;
    }

    if (nextOutboundChannelId < minId || nextOutboundChannelId > maxId)
        nextOutboundChannelId = minId;

    // Find an unused id, trying a maximum of 100 times
    for (int i = 0; i < 100 && channels.contains(nextOutboundChannelId); i++) {
        nextOutboundChannelId += qrand() % 200;
        if (nextOutboundChannelId > maxId)
            nextOutboundChannelId = minId;
    }

    if (channels.contains(nextOutboundChannelId)) {
        // Abort the connection if we still couldn't find an id, because it's probably a nasty bug
        BUG() << "Can't find an available outbound channel ID for connection; aborting connection";
        socket->abort();
        return -1;
    }

    if (nextOutboundChannelId < minId || nextOutboundChannelId > maxId) {
        BUG() << "Selected a channel id that isn't within range";
        return -1;
    }

    int re = nextOutboundChannelId;
    nextOutboundChannelId++;
    return re;
}

bool ConnectionPrivate::isValidAvailableChannelId(int id, Connection::Direction side)
{
    int minId, maxId;
    if (side == Connection::ClientSide) {
        minId = 1;
        maxId = 0x7fff;
    } else {
        minId = 0x8000;
        maxId = 0xffff;
    }

    if (id < minId || id > maxId)
        return false;

    if (channels.contains(id))
        return false;

    return true;
}

bool ConnectionPrivate::insertChannel(Channel *channel)
{
    if (channel->connection() != q) {
        BUG() << "Connection tried to insert a channel assigned to a different connection";
        return false;
    }

    if (channel->identifier() < 0) {
        BUG() << "Connection tried to insert a channel without a valid identifier";
        return false;
    }

    if (channels.contains(channel->identifier())) {
        BUG() << "Connection tried to insert a channel with a duplicate id" << channel->identifier()
              << "- we have" << channels.value(channel->identifier()) << "and inserted" << channel;
        return false;
    }

    if (channel->parent() != q) {
        BUG() << "Connection inserted a channel without expected parent object. Fixing.";
        channel->setParent(q);
    }

    channels.insert(channel->identifier(), channel);
    return true;
}

void ConnectionPrivate::removeChannel(Channel *channel)
{
    if (channel->connection() != q) {
        BUG() << "Connection tried to remove a channel assigned to a different connection";
        return;
    }

    // Out of caution, find the channel by pointer instead of identifier. This will make sure
    // it's always removed from the list, even if the identifier was somehow reset or lost.
    for (auto it = channels.begin(); it != channels.end(); ) {
        if (*it == channel)
            it = channels.erase(it);
        else
            it++;
    }
}

void ConnectionPrivate::closeAllChannels()
{
    // Takes a copy, won't be broken by removeChannel calls
    foreach (Channel *channel, channels)
        channel->closeChannel();

    if (!channels.isEmpty())
        BUG() << "Channels remain open on connection after calling closeAllChannels";
}

QHash<int,Channel*> Connection::channels()
{
    return d->channels;
}

Channel *Connection::channel(int identifier)
{
    return d->channels.value(identifier);
}

bool Connection::hasAuthenticated(AuthenticationType type) const
{
    return d->authentication.contains(type);
}

bool Connection::hasAuthenticatedAs(AuthenticationType type, const QString &identity) const
{
    auto it = d->authentication.find(type);
    if (!identity.isEmpty() && it != d->authentication.end())
        return *it == identity;
    return false;
}

QString Connection::authenticatedIdentity(AuthenticationType type) const
{
    return d->authentication.value(type);
}

void Connection::grantAuthentication(AuthenticationType type, const QString &identity)
{
    if (hasAuthenticated(type)) {
        BUG() << "Tried to redundantly grant" << type << "authentication to connection";
        return;
    }

    qDebug() << "Granting" << type << "authentication as" << identity << "to connection";

    d->authentication.insert(type, identity);
    emit authenticated(type, identity);
}

