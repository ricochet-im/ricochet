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
#include "ControlChannel.h"
#include "utils/Useful.h"

using namespace Protocol;

Connection::Connection(QTcpSocket *socket, Direction direction)
    : QObject()
    , d(new ConnectionPrivate(this))
{
    d->setSocket(socket, direction);
}

ConnectionPrivate::ConnectionPrivate(Connection *qq)
    : QObject(qq)
    , q(qq)
    , socket(0)
    , direction(Connection::ClientSide)
    , purpose(Connection::Purpose::Unknown)
    , wasClosed(false)
    , handshakeDone(false)
    , nextOutboundChannelId(-1)
{
    ageTimer.start();

    QTimer *timeout = new QTimer(this);
    timeout->setSingleShot(true);
    timeout->setInterval(UnknownPurposeTimeout * 1000);
    connect(timeout, &QTimer::timeout, this,
        [this,timeout]() {
            if (purpose == Connection::Purpose::Unknown) {
                qDebug() << "Closing connection" << q << "with unknown purpose after timeout";
                q->close();
            }
            timeout->deleteLater();
        }
    );
    timeout->start();
}

Connection::~Connection()
{
    qDebug() << this << "Destroying connection";

    // When we call closeImmediately, the list of channels will be cleared.
    // In the normal case, they will all use deleteLater to be freed at the
    // next event loop. Since the connection is being destructed immediately,
    // and we want to be certain that channels don't outlive it, copy the
    // list before it's cleared and delete them immediately afterwards.
    auto channels = d->channels;
    d->closeImmediately();

    // These would be deleted by QObject ownership as well, but we want to
    // give them a chance to destruct before the connection d pointer is reset.
    foreach (Channel *c, channels)
        delete c;

    // Reset d pointer, so we'll crash nicely if anything tries to call
    // into Connection after this.
    d = 0;
}

ConnectionPrivate::~ConnectionPrivate()
{
    // Reset q pointer, for the same reason as above
    q = 0;
}

Connection::Direction Connection::direction() const
{
    return d->direction;
}

bool Connection::isConnected() const
{
    bool re = d->socket && d->socket->state() == QAbstractSocket::ConnectedState;
    if (d->wasClosed) {
        Q_ASSERT(!re);
    }
    return re;
}

QString Connection::serverHostname() const
{
    QString hostname;
    if (direction() == ClientSide)
        hostname = d->socket->peerName();
    else if (direction() == ServerSide)
        hostname = d->socket->property("localHostname").toString();

    if (!hostname.endsWith(QStringLiteral(".onion"))) {
        TEGO_BUG() << "Connection does not have a valid server hostname:" << hostname;
        return QString();
    }

    return hostname;
}

QByteArray Connection::serverServiceId() const
{
    auto hostname = this->serverHostname();
    return hostname.toUtf8().left(TEGO_V3_ONION_SERVICE_ID_LENGTH);
}

int Connection::age() const
{
    return qRound(static_cast<double>(d->ageTimer.elapsed()) / 1000.0);
}

void ConnectionPrivate::setSocket(QTcpSocket *s, Connection::Direction d)
{
    if (socket) {
        TEGO_BUG() << "Connection already has a socket";
        return;
    }

    socket = s;
    direction = d;
    connect(socket, &QAbstractSocket::disconnected, this, &ConnectionPrivate::socketDisconnected);
    connect(socket, &QIODevice::readyRead, this, &ConnectionPrivate::socketReadable);

    socket->setParent(q);

    if (socket->state() != QAbstractSocket::ConnectedState) {
        TEGO_BUG() << "Connection created with socket in a non-connected state" << socket->state();
    }

    Channel *control = new ControlChannel(direction == Connection::ClientSide ? Channel::Outbound : Channel::Inbound, q);
    // Closing the control channel must also close the connection
    connect(control, &Channel::invalidated, q, &Connection::close);
    insertChannel(control);

    if (!control->isOpened() || control->identifier() != 0 || q->channel(0) != control) {
        TEGO_BUG() << "Control channel on new connection is not set up properly";
        q->close();
        return;
    }

    if (direction == Connection::ClientSide) {
        // The server side is implicitly authenticated (by the transport) as the correct service, so grant that
        QString serverName = q->serverHostname();
        if (serverName.isEmpty()) {
            TEGO_BUG() << "Server side of connection doesn't have an authenticated name, aborting";
            q->close();
            return;
        }

        q->grantAuthentication(Connection::HiddenServiceAuth, serverName);

        // Send the introduction version handshake message
        char intro[] = { 0x49, 0x4D, 0x01, ProtocolVersion };
        if (socket->write(intro, sizeof(intro)) < static_cast<int>(sizeof(intro))) {
            qDebug() << "Failed writing introduction message to socket";
            q->close();
            return;
        }
    }
}

void Connection::close()
{
    if (isConnected()) {
        Q_ASSERT(!d->wasClosed);
        qDebug() << "Disconnecting socket for connection" << this;
        d->socket->disconnectFromHost();

        // If not fully closed in 5 seconds, abort
        QTimer *timeout = new QTimer(this);
        timeout->setSingleShot(true);
        connect(timeout, &QTimer::timeout, d, &ConnectionPrivate::closeImmediately);
        timeout->start(5000);
    }
}

void ConnectionPrivate::closeImmediately()
{
    if (socket)
        socket->abort();

    if (!wasClosed) {
        TEGO_BUG() << "Socket was forcefully closed but never emitted closed signal";
        wasClosed = true;
        emit q->closed();
    }

    if (!channels.isEmpty()) {
        foreach (Channel *c, channels)
            qDebug() << "Open channel:" << c << c->type() << c->connection();
        TEGO_BUG() << "Channels remain open after forcefully closing connection socket";
    }
}

void ConnectionPrivate::socketDisconnected()
{
    qDebug() << "Connection" << this << "disconnected";
    // emit close signal first so FileChannel can bubble up errors
    if (!wasClosed) {
        wasClosed = true;
        emit q->closed();
    }

    closeAllChannels();
}

void ConnectionPrivate::socketReadable()
{
    if (!handshakeDone) {
        qint64 available = socket->bytesAvailable();

        if (direction == Connection::ClientSide && available >= 1) {
            // Expecting a single byte in response with the chosen version
            uchar version = ProtocolVersionFailed;
            if (socket->read(reinterpret_cast<char*>(&version), 1) < 1) {
                qDebug() << "Connection socket error" << socket->error() << "during read:" << socket->errorString();
                socket->abort();
                return;
            }

            handshakeDone = true;
            if (version == 0) {
                qDebug() << "Server in outbound connection is using the version 1.0 protocol";
                emit q->oldVersionNegotiated(socket);
                q->close();
                return;
            } else if (version != ProtocolVersion) {
                qDebug() << "Version negotiation failed on outbound connection";
                emit q->versionNegotiationFailed();
                socket->abort();
                return;
            } else
                emit q->ready();
        } else if (direction == Connection::ServerSide && available >= 3) {
            // Expecting at least 3 bytes
            uchar intro[3] = { 0 };
            qint64 re = socket->peek(reinterpret_cast<char*>(intro), sizeof(intro));
            if (re < static_cast<int>(sizeof(intro))) {
                qDebug() << "Connection socket error" << socket->error() << "during read:" << socket->errorString();
                socket->abort();
                return;
            }

            quint8 nVersions = intro[2];
            if (intro[0] != 0x49 || intro[1] != 0x4D || nVersions == 0) {
                qDebug() << "Invalid introduction sequence on inbound connection";
                socket->abort();
                return;
            }

            if (available < static_cast<qint64>(sizeof(intro)) + nVersions)
                return;

            // Discard intro header
            re = socket->read(reinterpret_cast<char*>(intro), sizeof(intro));
            (void)re;

            QByteArray versions(nVersions, 0);
            re = socket->read(versions.data(), versions.size());
            if (re != versions.size()) {
                qDebug() << "Connection socket error" << socket->error() << "during read:" << socket->errorString();
                socket->abort();
                return;
            }

            quint8 selectedVersion = ProtocolVersionFailed;
            for (auto v : versions) {
                if (static_cast<quint8>(v) == ProtocolVersion) {
                    selectedVersion = static_cast<quint8>(v);
                    break;
                }
            }

            re = socket->write(reinterpret_cast<char*>(&selectedVersion), 1);
            if (re != 1) {
                qDebug() << "Connection socket error" << socket->error() << "during write:" << socket->errorString();
                socket->abort();
                return;
            }

            handshakeDone = true;
            if (selectedVersion != ProtocolVersion) {
                qDebug() << "Version negotiation failed on inbound connection";
                emit q->versionNegotiationFailed();
                // Close gracefully to allow the response to write
                q->close();
                return;
            } else
                emit q->ready();
        } else {
            return;
        }
    }

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
            TEGO_BUG() << "Socket had" << available << "bytes available but peek only returned" << re;
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
                TEGO_BUG() << "Socket read was unexpectedly small;" << available << "bytes should've been available but we read" << re;
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
                TEGO_BUG() << "Socket read was unexpectedly small;" << available << "bytes should've been available but we read" << re;
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
            TEGO_BUG() << "Channel" << channelId << "found on connection" << this << "but its connection is"
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
        TEGO_BUG() << "Writing packet for channel" << channel->identifier() << "on connection" << this
              << "but its connection is" << channel->connection();
        qFatal("Connection mismatch while writing packet");
        return false;
    }

    return writePacket(channel->identifier(), data);
}

bool ConnectionPrivate::writePacket(int channelId, const QByteArray &data)
{
    if (channelId < 0 || channelId > UINT16_MAX) {
        TEGO_BUG() << "Cannot write packet for channel with invalid identifier" << channelId;
        return false;
    }

    if (data.size() > PacketMaxDataSize) {
        TEGO_BUG() << "Cannot write oversized packet of" << data.size() << "bytes to channel" << channelId;
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

    re = socket->write(data, data.size());
    if (re != data.size()) {
        qDebug() << "Connection socket error" << socket->error() << "during write:" << socket->errorString();
        socket->abort();
        return false;
    }

    return true;
}

int ConnectionPrivate::availableOutboundChannelId()
{
    // Server opens even-nubmered channels, client opens odd-numbered
    bool evenNumbered = (direction == Connection::ServerSide);
    const int minId = evenNumbered ? 2 : 1;
    const int maxId = evenNumbered ? (UINT16_MAX-1) : UINT16_MAX;

    if (nextOutboundChannelId < minId || nextOutboundChannelId > maxId)
        nextOutboundChannelId = minId;

    // Find an unused id, trying a maximum of 100 times, using a random step to avoid collision
    for (int i = 0; i < 100 && channels.contains(nextOutboundChannelId); i++) {
        nextOutboundChannelId += 1 + (QRandomGenerator::global()->bounded(200));
        if (evenNumbered)
            nextOutboundChannelId += nextOutboundChannelId % 2;
        if (nextOutboundChannelId > maxId)
            nextOutboundChannelId = minId;
    }

    if (channels.contains(nextOutboundChannelId)) {
        // Abort the connection if we still couldn't find an id, because it's probably a nasty bug
        TEGO_BUG() << "Can't find an available outbound channel ID for connection; aborting connection";
        socket->abort();
        return -1;
    }

    if (nextOutboundChannelId < minId || nextOutboundChannelId > maxId) {
        TEGO_BUG() << "Selected a channel id that isn't within range";
        return -1;
    }

    if (evenNumbered == bool(nextOutboundChannelId % 2)) {
        TEGO_BUG() << "Selected a channel id that isn't valid for this side of the connection";
        return -1;
    }

    int re = nextOutboundChannelId;
    nextOutboundChannelId += 2;
    return re;
}

bool ConnectionPrivate::isValidAvailableChannelId(int id, Connection::Direction side)
{
    if (id < 1 || id > UINT16_MAX)
        return false;

    bool evenNumbered = bool(id % 2);
    if (evenNumbered == (side == Connection::ServerSide))
        return false;

    if (channels.contains(id))
        return false;

    return true;
}

bool ConnectionPrivate::insertChannel(Channel *channel)
{
    if (channel->connection() != q) {
        TEGO_BUG() << "Connection tried to insert a channel assigned to a different connection";
        return false;
    }

    if (channel->identifier() < 0) {
        TEGO_BUG() << "Connection tried to insert a channel without a valid identifier";
        return false;
    }

    if (channels.contains(channel->identifier())) {
        TEGO_BUG() << "Connection tried to insert a channel with a duplicate id" << channel->identifier()
              << "- we have" << channels.value(channel->identifier()) << "and inserted" << channel;
        return false;
    }

    if (channel->parent() != q) {
        TEGO_BUG() << "Connection inserted a channel without expected parent object. Fixing.";
        channel->setParent(q);
    }

    channels.insert(channel->identifier(), channel);
    return true;
}

void ConnectionPrivate::removeChannel(Channel *channel)
{
    if (channel->connection() != q) {
        TEGO_BUG() << "Connection tried to remove a channel assigned to a different connection";
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
        TEGO_BUG() << "Channels remain open on connection after calling closeAllChannels";
}

QHash<int,Channel*> Connection::channels()
{
    return d->channels;
}

Channel *Connection::channel(int identifier)
{
    return d->channels.value(identifier);
}

Connection::Purpose Connection::purpose() const
{
    return d->purpose;
}

bool Connection::setPurpose(Purpose value)
{
    if (d->purpose == value)
        return true;

    switch (value) {
        case Purpose::Unknown:
            TEGO_BUG() << "A connection can't reset to unknown purpose";
            return false;
        case Purpose::KnownContact:
            if (!hasAuthenticated(HiddenServiceAuth)) {
                TEGO_BUG() << "Connection purpose cannot be KnownContact without authenticating a service";
                return false;
            }
            break;
        case Purpose::OutboundRequest:
            if (d->direction != ClientSide) {
                TEGO_BUG() << "Connection purpose cannot be OutboundRequest on an inbound connection";
                return false;
            } else if (d->purpose != Purpose::Unknown) {
                TEGO_BUG() << "Connection purpose cannot change from" << int(d->purpose) << "to OutboundRequest";
                return false;
            }
            break;
        case Purpose::InboundRequest:
            if (d->direction != ServerSide) {
                TEGO_BUG() << "Connection purpose cannot be InboundRequest on an outbound connection";
                return false;
            } else if (d->purpose != Purpose::Unknown) {
                TEGO_BUG() << "Connection purpose cannot change from" << int(d->purpose) << "to InboundRequest";
                return false;
            }
            break;
        default:
            TEGO_BUG() << "Purpose type" << int(value) << "is not defined";
            return false;
    }

    Purpose old = d->purpose;
    d->purpose = value;
    emit purposeChanged(d->purpose, old);
    return true;
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
        TEGO_BUG() << "Tried to redundantly grant" << type << "authentication to connection";
        return;
    }

    qDebug() << "Granting" << type << "authentication as" << identity << "to connection";

    d->authentication.insert(type, identity);
    emit authenticated(type, identity);
}

