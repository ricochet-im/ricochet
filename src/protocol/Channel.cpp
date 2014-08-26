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

using namespace Protocol;

Channel *Channel::create(const QString &type, Direction direction, QObject *parent)
{
    qFatal("XXX not implemented");
    return 0;
}

Channel::Channel(const QString &type, Direction direction, QObject *parent)
    : QObject(parent)
    , d_ptr(new ChannelPrivate(this, type, direction))
{
}

Channel::Channel(ChannelPrivate *d_ptr, QObject *parent)
    : QObject(parent)
    , d_ptr(d_ptr)
{
}

Channel::~Channel()
{
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
    Q_ASSERT(d->identifier <= UINT16_MAX);
    return d->identifier;
}

Channel::Direction Channel::direction() const
{
    Q_D(const Channel);
    return d->direction;
}

bool Channel::isOpened() const
{
    Q_D(const Channel);
    if (d->isOpened && d->identifier < 0)
        BUG() << "Channel is marked as open, but has no identifier";
    return d->isOpened;
}

void Channel::closeChannel()
{
    qFatal("XXX not implemented");
}

/* Called by ControlChannel to handle an inbound OpenChannel message.
 * This Channel must be in a clean, inbound state. The Channel subclass
 * decides whether to accept the request, and can add data to the result.
 */
bool ChannelPrivate::openChannelInbound(const Data::Control::OpenChannel *request, Data::Control::ChannelResult *result)
{
    qFatal("XXX not implemented");
    return false;
}

bool ChannelPrivate::openChannelOutbound(Data::Control::OpenChannel *request)
{
    qFatal("XXX not implemented");
    return false;
}

bool ChannelPrivate::openChannelResult(const Data::Control::ChannelResult *result)
{
    qFatal("XXX not implemented");
    return false;
}

bool Channel::processChannelOpenResult(const Data::Control::ChannelResult *result)
{
    Q_UNUSED(result);
    return true;
}

bool Channel::sendPacket(const QByteArray &packet)
{
    qFatal("XXX not implemented");
    return false;
}

ChannelPrivate::ChannelPrivate(Channel *q, const QString &type, Channel::Direction direction)
    : q_ptr(q)
    , type(type)
    , identifier(-1)
    , direction(direction)
    , isOpened(false)
{
}

ChannelPrivate::~ChannelPrivate()
{
}
