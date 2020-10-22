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

#ifndef PROTOCOL_CONNECTION_H
#define PROTOCOL_CONNECTION_H

#include "Channel.h"

class QTcpSocket;

namespace Protocol
{

class ConnectionPrivate;

/* Represents a protocol connection associated with a socket
 *
 * Connection is created to handle protocol communication over a socket. It
 * handles reading, writing, creating channels, and all protocol behavior. A
 * connection instance is created for a specific socket and cannot be changed.
 * When the socket is closed, the Connection closes all channels and cannot be
 * used again.
 *
 * All protocol behavior takes place by creating and using channels, represented
 * by subclasses of Channel. The channelCreated and channelOpened signals can be
 * used to attach to new channels. A new channel can be created by instantiating
 * it and calling its openChannel method.
 *
 * The socket and all channels are owned by the Connection instance. In
 * particular, channel instances will be deleted automatically after being
 * closed. Avoid storing pointers to channels, or use a safe pointer to do so.
 *
 * The channel's functionality is controlled by authentication grants and by its
 * assigned purpose. The purpose declares the current use of the channel (e.g.
 * for a known contact or an incoming contact request). Higher level classes
 * assign and change the connection's purpose. Connections with an Unknown
 * purpose are closed automatically after a short timeout.
 */
class Connection : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Connection)

    friend class Channel;
    friend class ChannelPrivate;
    friend class ControlChannel;

public:
    /* Direction of the underlying socket connection
     *
     * The protocol is peer-to-peer and doesn't differentiate between
     * server and client except for small behavior details.
     */
    enum Direction {
        ClientSide,
        ServerSide
    };

    /* Construct a connection handler for a socket
     *
     * This connection will take ownership of the socket, and
     * becomes invalid (but is not automatically deleted) once
     * the socket has disconnected.
     */
    explicit Connection(QTcpSocket *socket, Direction direction);
    virtual ~Connection();

    Direction direction() const;
    bool isConnected() const;

    /* Hostname of the server side of the connection
     *
     * For a ClientSide connection, this returns the hostname that
     * the socket has connected to. For a ServerSide connection,
     * the local hostname which accepted the socket is returned.
     *
     * In all cases, the returned hostname will end with ".onion"
     */
    QString serverHostname() const;

    /* Age of the connection in seconds */
    int age() const;

    /* Assigned purpose of this connection
     *
     * A purpose is assigned to the connection after the peer has
     * authenticated or otherwise indicated what the connection will
     * be used for.
     *
     * Purposes may be used to limit the features available on a
     * connection, change behavior, and impose restrictions.
     *
     * Connections with an unknown purpose are killed after a timeout.
     */
    enum class Purpose {
        Unknown,
        KnownContact,
        OutboundRequest,
        InboundRequest
    };

    Purpose purpose() const;
    bool setPurpose(Purpose purpose);

    QHash<int,Channel*> channels();
    Channel *channel(int identifier);
    template<typename T> T *findChannel(Channel::Direction direction = Channel::Invalid);
    template<typename T> QList<T*> findChannels(Channel::Direction direction = Channel::Invalid);

    enum AuthenticationType {
        HiddenServiceAuth,
        KnownToPeer // For outbound connections, set when the peer indicates we are a known contact
    };

    bool hasAuthenticated(AuthenticationType type) const;
    bool hasAuthenticatedAs(AuthenticationType type, const QString &identity) const;
    QString authenticatedIdentity(AuthenticationType type) const;
    void grantAuthentication(AuthenticationType type, const QString &identity = QString());

public slots:
    /* Close this connection and the underlying socket
     *
     * All pending data to write will be sent, and the socket will be
     * asynchronously closed. If data hasn't been written after 5 seconds, the
     * socket will timeout and close anyway.
     *
     * isConnected will return false immediately after this function is called.
     * The closed signal is emitted when the socket and all channels have closed.
     */
    void close();

signals:
    /* Emitted when the socket is closed. All channels will be closed
     * automatically. It is not possible to re-use the same Connection instance,
     * or to reconnect the socket.
     */
    void closed();
    /* Emitted once, after version negotiation has finished and the connection
     * is ready to use. If negotiation fails, the versionNegotiationFailed
     * signal is emitted instead, and the socket is closed.
     */
    void ready();
    /* Emitted once when version negotiation has failed; meaning, there is no
     * protocol version that both peers will accept. The socket will be closed.
     */
    void versionNegotiationFailed();
    /* Hack to allow delivering an upgrade message to old clients
     * XXX: Remove this once enough time has passed for most clients to be upgraded.
     */
    void oldVersionNegotiated(QTcpSocket *socket);

    void authenticated(AuthenticationType type, const QString &identity);
    void purposeChanged(Purpose after, Purpose before);
    /* Emitted when a new Channel instance is created, before it has opened
     *
     * This signal can be used to attach to signals on a channel before it's
     * opened. This signal is emitted for both inbound and outbound channels,
     * before the request is approved. If a request is rejected or fails, the
     * channel may be deleted shortly afterwards, without emitting its
     * channelClosed signal.
     */
    void channelCreated(Channel *channel);
    /* Emitted when an inbound channel needs approval to open
     *
     * This signal is emitted for channel types that require approval by
     * higher-layer functionality before opening, based on the information
     * in the OpenChannel message. Handlers should use channel-specific methods
     * to approve the inbound channel.
     *
     * This signal is only emitted for channels that specifically invoke the
     * Channel::requestInboundApproval() method.
     */
    void channelRequestingInboundApproval(Channel *channel);
    /* Emitted when a channel is opened
     *
     * This signal is emitted after an inbound or outbound channel has been
     * opened. At this point, the channel can be used or closed normally.
     */
    void channelOpened(Channel *channel);

private:
    ConnectionPrivate *d;
};

template<typename T> T *Connection::findChannel(Channel::Direction direction)
{
    T *re = 0;
    foreach (Channel *c, channels()) {
        if (direction != Channel::Invalid && c->direction() != direction)
            continue;
        if ((re = qobject_cast<T*>(c)))
            return re;
    }
    return 0;
}

template<typename T> QList<T*> Connection::findChannels(Channel::Direction direction)
{
    QList<T*> re;
    T *tmp = 0;
    foreach (Channel *c, channels()) {
        if (direction != Channel::Invalid && c->direction() != direction)
            continue;
        if ((tmp = qobject_cast<T*>(c)))
            re.append(tmp);
    }
    return re;
}

}

#endif
