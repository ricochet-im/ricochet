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

#include <QObject>
#include <QHash>
#include "Channel.h"

class QTcpSocket;

namespace Protocol
{

class ConnectionPrivate;

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
    explicit Connection(QTcpSocket *socket, Direction direction, QObject *parent);

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
        KnownContact
    };

    Purpose purpose() const;
    bool setPurpose(Purpose purpose);

    QHash<int,Channel*> channels();
    Channel *channel(int identifier);
    template<typename T> T *findChannel(Channel::Direction direction = Channel::Invalid);
    template<typename T> QList<T*> findChannels(Channel::Direction direction = Channel::Invalid);

    enum AuthenticationType {
        HiddenServiceAuth
    };

    bool hasAuthenticated(AuthenticationType type) const;
    bool hasAuthenticatedAs(AuthenticationType type, const QString &identity) const;
    QString authenticatedIdentity(AuthenticationType type) const;
    void grantAuthentication(AuthenticationType type, const QString &identity = QString());

public slots:
    void close();

signals:
    void closed();
    void authenticated(AuthenticationType type, const QString &identity);
    void purposeChanged(Purpose after, Purpose before);
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
