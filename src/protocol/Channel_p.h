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

#ifndef PROTOCOL_CHANNEL_P_H
#define PROTOCOL_CHANNEL_P_H

#include "Channel.h"
#include "Connection_p.h"
#include "utils/Useful.h"
#include <QDebug>
#include "logger.hpp"

namespace Protocol
{

class ChannelPrivate : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ChannelPrivate)
    Q_DECLARE_PUBLIC(Channel)

public:
    explicit ChannelPrivate(Channel *q, const QString &type, Channel::Direction direction, Connection *conn);
    virtual ~ChannelPrivate();

    Channel *q_ptr;
    Connection *connection;
    QString type;
    int identifier;
    Channel::Direction direction;
    bool isOpened;
    bool hasSentClose;
    bool isInvalidated;

    void invalidate();

    // Called by ControlChannel to act on valid channel request/result messages
    bool openChannelInbound(const Data::Control::OpenChannel *request, Data::Control::ChannelResult *result);
    bool openChannelOutbound(Data::Control::OpenChannel *request);
    bool openChannelResult(const Data::Control::ChannelResult *result);
};

template<typename T> bool Channel::sendMessage(const T &message)
{
    int size = message.ByteSize();
    if (size > ConnectionPrivate::PacketMaxDataSize) {
        BUG() << "Message on" << type() << "channel is too big -" << size << "bytes:"
              << QString::fromStdString(message.DebugString());
        return false;
    }

    if (size < 1) {
        BUG() << "Message on" << type() << "channel encoded as invalid length; this isn't possible to send:"
              << QString::fromStdString(message.DebugString());
        return false;
    }

    QByteArray packet(size, 0);
    quint8 *end = message.SerializeWithCachedSizesToArray(reinterpret_cast<quint8*>(packet.data()));
    quint8 *expected_end = reinterpret_cast<quint8*>(packet.data() + size);
    if (end != expected_end) {
        BUG() << "Unexpected packet size after message serialization. Expected" << size << "but got" << qptrdiff(end - expected_end);
        return false;
    }

    logger::println("send {}\n{}", typeid(T), packet);

    return sendPacket(packet);
}

}

#endif
