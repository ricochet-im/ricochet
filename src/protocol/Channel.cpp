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

#include "Channel_p.h"
#include "Connection_p.h"
#include "ControlChannel.h"
#include "utils/Useful.h"
#include <QDebug>

#include "AuthHiddenServiceChannel.h"
#include "ChatChannel.h"

using namespace Protocol;

Channel *Channel::create(const QString &type, Direction direction, Connection *connection)
{
    if (!connection)
        return 0;

    if (type == QStringLiteral("im.ricochet.auth.hidden-service")) {
        return new AuthHiddenServiceChannel(direction, connection);
    } else if (type == QStringLiteral("im.ricochet.chat")) {
        return new ChatChannel(direction, connection);
    } else {
        return 0;
    }
}

Channel::Channel(const QString &type, Direction direction, Connection *connection)
    : QObject(connection)
    , d_ptr(new ChannelPrivate(this, type, direction, connection))
{
}

Channel::Channel(ChannelPrivate *d_ptr)
    : QObject(d_ptr->connection)
    , d_ptr(d_ptr)
{
}

Channel::~Channel()
{
    Q_D(Channel);
    if (d->identifier >= 0 && !d->isInvalidated)
        d->connection->d->removeChannel(this);
}

QString Channel::type() const
{
    Q_D(const Channel);
    return d->type;
}

// May return -1 for unassigned channels
int Channel::identifier() const
{
    Q_D(const Channel);
    if (d->identifier > UINT16_MAX)
        return -1;
    return d->identifier;
}

Channel::Direction Channel::direction() const
{
    Q_D(const Channel);
    return d->direction;
}

Connection *Channel::connection()
{
    Q_D(Channel);
    return d->connection;
}

bool Channel::isOpened() const
{
    Q_D(const Channel);
    if (d->isOpened && d->identifier < 0)
        BUG() << "Channel is marked as open, but has no identifier";
    return d->isOpened;
}

bool Channel::openChannel()
{
    Q_D(Channel);
    if (direction() != Channel::Outbound || isOpened() || identifier() >= 0) {
        BUG() << "Cannot send request to open" << type() << "channel in an incorrect state";
        if (isOpened())
            closeChannel();
        d->invalidate();
        return false;
    } else if (!connection()->findChannel<ControlChannel>()->sendOpenChannel(this)) {
        if (isOpened()) {
            BUG() << "Channel somehow opened instantly in an impossible situation";
            closeChannel();
        }
        d->invalidate();
        return false;
    }

    return true;
}

void Channel::closeChannel()
{
    Q_D(Channel);

    if (!d->hasSentClose && d->identifier >= 0 && connection()->isConnected()) {
        d->hasSentClose = true;
        bool ok = connection()->d->writePacket(this, QByteArray());
        if (!ok)
            qDebug() << "Failed sending channel close message";
    }

    // Invalidate will remove and eventually destroy the Channel
    d->isOpened = false;
    d->invalidate();
}

/* Called by ControlChannel to handle an inbound OpenChannel message.
 * This Channel must be in a clean, inbound state. The Channel subclass
 * decides whether to accept the request, and can add data to the result.
 */
bool ChannelPrivate::openChannelInbound(const Data::Control::OpenChannel *request, Data::Control::ChannelResult *result)
{
    Q_Q(Channel);
    result->set_opened(false);
    if (direction != Channel::Inbound || isOpened || identifier >= 0 || hasSentClose || isInvalidated) {
        BUG() << "Handling inbound open channel request on a channel in an unexpected state; rejecting";
        return false;
    }

    if (request->channel_identifier() <= 0) {
        BUG() << "Invalid channel identifier in inboundOpenChannel handler";
        return false;
    }

    // The Connection::channelCreated signal must emit once, after the Channel
    // is fully constructed, but before it's used. This is that moment: just
    // before we check whether to allow an inbound/outbound request.
    emit connection->channelCreated(q);

    if (!q->allowInboundChannelRequest(request, result))
        return false;

    if (result->has_common_error() || result->has_error_message()) {
        BUG() << "Accepted inbound OpenChannel request, but result has error details set. Assuming it's actually an error.";
        result->set_opened(false);
        return false;
    }

    result->set_opened(true);
    identifier = request->channel_identifier();
    isOpened = true;
    emit q->channelOpened();
    return true;
}

bool ChannelPrivate::openChannelOutbound(Data::Control::OpenChannel *request)
{
    Q_Q(Channel);
    if (direction != Channel::Outbound || isOpened || identifier >= 0) {
        BUG() << "Handling outbound open channel request on a channel in an unexpected state; rejecting";
        return false;
    }

    // The Connection::channelCreated signal must emit once, after the Channel
    // is fully constructed, but before it's used. This is that moment: just
    // before we check whether to allow an inbound/outbound request.
    emit connection->channelCreated(q);

    if (!q->allowOutboundChannelRequest(request))
        return false;

    request->set_channel_type(type.toStdString());
    identifier = request->channel_identifier();
    return true;
}

bool ChannelPrivate::openChannelResult(const Data::Control::ChannelResult *result)
{
    Q_Q(Channel);
    // ControlChannel should weed out clearly invalid messages, so assert here if it didn't
    if (direction != Channel::Outbound || isOpened || identifier < 0) {
        BUG() << "Handling response for outbound open channel on a channel in an unexpected state; ignoring";
        return false;
    }

    bool ok = result->opened();
    if (!q->processChannelOpenResult(result)) {
        // If the peer thinks the channel was opened successfully, send a close
        if (result->opened())
            q->closeChannel();
        ok = false;
    }

    if (ok) {
        isOpened = true;
        emit q->channelOpened();
    } else {
        Data::Control::ChannelResult::CommonError error = Data::Control::ChannelResult::GenericError;
        if (result->has_common_error())
            error = result->common_error();
        emit q->channelRejected(error, QString::fromStdString(result->error_message()));
        invalidate();
    }

    return ok;
}

bool Channel::processChannelOpenResult(const Data::Control::ChannelResult *result)
{
    Q_UNUSED(result);
    return true;
}

bool Channel::sendPacket(const QByteArray &packet)
{
    Q_D(Channel);
    if (d->identifier < 0) {
        BUG() << "Cannot send packet to channel" << type() << "without an assigned identifier";
        return false;
    }

    if (packet.size() == 0) {
        BUG() << "Cannot send empty packet to channel" << type();
        return false;
    }

    if (packet.size() > ConnectionPrivate::PacketMaxDataSize) {
        BUG() << "Packet is too big on channel" << type();
        return false;
    }

    return connection()->d->writePacket(this, packet);
}

ChannelPrivate::ChannelPrivate(Channel *q, const QString &type, Channel::Direction direction, Connection *conn)
    : q_ptr(q)
    , connection(conn)
    , type(type)
    , identifier(-1)
    , direction(direction)
    , isOpened(false)
    , hasSentClose(false)
    , isInvalidated(false)
{
}

ChannelPrivate::~ChannelPrivate()
{
    Q_Q(Channel);
    if (identifier >= 0 && !isInvalidated) {
        BUG() << "Channel of type" << type << "was deleted without being invalidated";
        connection->d->removeChannel(q);
    }
}

void ChannelPrivate::invalidate()
{
    Q_Q(Channel);
    if (isInvalidated)
        return;

    Q_ASSERT(!isOpened);

    qDebug() << "Invalidating channel" << q << "type" << type << "id" << identifier;

    isInvalidated = true;
    emit q->invalidated();

    if (identifier >= 0) {
        connection->d->removeChannel(q);
        Q_ASSERT(!connection->channel(identifier));
    }

    q->deleteLater();
}
