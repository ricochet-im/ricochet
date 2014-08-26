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

#include "ControlChannel.h"
#include "Channel_p.h"
#include "utils/Useful.h"
#include <QDebug>

using namespace Protocol;

ControlChannel::ControlChannel(Direction direction, QObject *parent)
    : Channel(QStringLiteral("control"), direction, parent)
{
}

bool ControlChannel::openChannel(Channel *channel)
{
    if (channel->isOpened() || channel->direction() != Outbound || channel->identifier() >= 0) {
        BUG() << "openChannel called for a" << channel->type() << "channel in an unexpected state";
        return false;
    }

    QScopedPointer<Data::Control::OpenChannel> request(new Data::Control::OpenChannel);
    // XXX Also, add to the connection's list
    qCritical("XXX I'm supposed to set the identifier here");

    if (!channel->d_ptr->openChannelOutbound(request.data())) {
        qDebug() << "Outbound OpenChannel request of type" << channel->type() << "refused locally";
        return false;
    }

    if (!request->has_channel_type() || !request->has_channel_identifier() ||
        request->channel_identifier() < 0 || request->channel_identifier() > UINT16_MAX)
    {
        BUG() << "Outbound OpenChannel request isn't valid:" << QString::fromStdString(request->DebugString());
        return false;
    }

    if (request->channel_identifier() != channel->identifier()) {
        BUG() << "Channel identifier doesn't match in OpenChannel request of type" << channel->type();
        return false;
    }

    Data::Control::Packet packet;
    packet.set_allocated_open_channel(request.take());
    return sendMessage(packet);
}

void ControlChannel::keepAlive()
{
    Data::Control::KeepAlive *request = new Data::Control::KeepAlive;
    request->set_response_requested(true);

    Data::Control::Packet packet;
    packet.set_allocated_keep_alive(request);
    sendMessage(packet);
}

bool ControlChannel::allowInboundChannelRequest(const Data::Control::OpenChannel *request, Data::Control::ChannelResult *result)
{
    Q_UNUSED(request);
    Q_UNUSED(result);
    BUG() << "ControlChannel should never receive channel requests";
    return false;
}

bool ControlChannel::allowOutboundChannelRequest(Data::Control::OpenChannel *request)
{
    Q_UNUSED(request);
    BUG() << "ControlChannel should never send channel requests";
    return false;
}

bool ControlChannel::processChannelOpenResult(const Data::Control::ChannelResult *result)
{
    Q_UNUSED(result);
    BUG() << "ControlChannel should never receive a channel request response";
    return false;
}

void ControlChannel::receivePacket(const QByteArray &packet)
{
    Data::Control::Packet message;
    if (!message.ParseFromArray(packet.constData(), packet.size())) {
        qWarning() << "Control channel failed parsing packet; connection will be killed";
        closeChannel();
        return;
    }

    if (message.has_open_channel()) {
        handleOpenChannel(message.open_channel());
    } else if (message.has_channel_result()) {
        handleChannelResult(message.channel_result());
    } else if (message.has_keep_alive()) {
        handleKeepAlive(message.keep_alive());
    } else {
        qWarning() << "Unrecognized message on control channel; connection will be killed";
        closeChannel();
        return;
    }
}

void ControlChannel::handleOpenChannel(const Data::Control::OpenChannel &message)
{
    // Validate channel_identifier
    int id = message.channel_identifier();
    // XXX client/server range rules
    // XXX identifier in use rules
    // XXX re-use rules
    if (id <= 0 || id > UINT16_MAX) {
        qWarning() << "Received OpenChannel with invalid channel_identifier:" << QString::fromStdString(message.DebugString());
        // Deliberately invalid behavior; kill the connection
        closeChannel();
        return;
    }

    Data::Control::ChannelResult *response = new Data::Control::ChannelResult;
    response->set_channel_identifier(id);

    Channel *channel = Channel::create(QString::fromStdString(message.channel_type()), Inbound, parent());
    if (!channel) {
        qDebug() << "Received OpenChannel for unknown channel type:" << QString::fromStdString(message.channel_type());
        response->set_opened(false);
        response->set_common_error(Data::Control::ChannelResult::UnknownTypeError);
    } else {
        if (!channel->d_ptr->openChannelInbound(&message, response)) {
            if (response->opened())
                BUG() << "openChannelInbound handler failed but response said successful. Assuming failure.";
            response->set_opened(false);
        }

        if (!response->has_opened()) {
            BUG() << "inboundOpenChannel handler for" << channel->type() << "did not update response message";
            response->set_opened(false);
            response->set_common_error(Data::Control::ChannelResult::GenericError);
        }
    }

    if (response->opened()) {
        if (!channel || !channel->isOpened() || channel->direction() != Inbound ||
            channel->identifier() != id)
        {
            BUG() << "Channel" << channel->type() << "in unexpected state after inbound open";
            response->set_opened(false);
            response->set_common_error(ControlChannelData::ChannelResult::GenericError);
            // The channel may think it's open, so force it to close
            channel->closeChannel();
        } else {
            // XXX Add channel to connection's list
            qCritical() << "XXX This should be implemented!";
        }
    }

    if (!response->opened()) {
        qDebug() << "Rejected OpenChannel request:" << QString::fromStdString(message.DebugString()) << "response:" << QString::fromStdString(response->DebugString());
        // Clean up channel instance
        delete channel;
        channel = 0;
    }

    Data::Control::Packet responseMessage;
    responseMessage.set_allocated_channel_result(response);
    sendMessage(responseMessage);
}

void ControlChannel::handleChannelResult(const Data::Control::ChannelResult &message)
{
    qFatal("XXX handleChannelResult not implemented");
}

void ControlChannel::handleKeepAlive(const Data::Control::KeepAlive &message)
{
    if (message.response_requested()) {
        Data::Control::KeepAlive *pong = new Data::Control::KeepAlive;
        pong->set_response_requested(false);
        Data::Control::Packet response;
        response.set_allocated_keep_alive(pong);
        sendMessage(response);
    } else {
        emit keepAliveResponse();
    }
}

