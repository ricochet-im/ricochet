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
#include "Connection_p.h"
#include "utils/Useful.h"

using namespace Protocol;

ControlChannel::ControlChannel(Direction direction, Connection *connection)
    : Channel(QStringLiteral("control"), direction, connection)
{
    if (connection->channel(0))
        BUG() << "Created ControlChannel for connection which already has a channel 0";

    Q_D(Channel);
    d->isOpened = true;
    d->identifier = 0;
}

bool ControlChannel::sendOpenChannel(Channel *channel)
{
    if (channel->isOpened() || channel->direction() != Outbound || channel->identifier() >= 0) {
        BUG() << "openChannel called for a" << channel->type() << "channel in an unexpected state";
        return false;
    }

    if (channel->connection() != connection()) {
        BUG() << "openChannel called for" << channel->type() << "channel on a different connection";
        return false;
    }

    QScopedPointer<Data::Control::OpenChannel> request(new Data::Control::OpenChannel);
    int channelId = connection()->d->availableOutboundChannelId();
    if (channelId <= 0)
        return false;
    request->set_channel_identifier(channelId);

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

    if (!connection()->d->insertChannel(channel)) {
        BUG() << "Valid channel refused by connection";
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

    logger::println("receive {}\n{}", type_name(message), message.DebugString());

    if (message.has_open_channel()) {
        handleOpenChannel(message.open_channel());
    } else if (message.has_channel_result()) {
        handleChannelResult(message.channel_result());
    } else if (message.has_keep_alive()) {
        handleKeepAlive(message.keep_alive());
    } else if (message.has_enable_features()) {
        handleEnableFeatures(message.enable_features());
    } else if (message.has_features_enabled()) {
        handleFeaturesEnabled(message.features_enabled());
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
    Connection::Direction peerSide = (connection()->direction() == Connection::ClientSide) ? Connection::ServerSide : Connection::ClientSide;
    if (!connection()->d->isValidAvailableChannelId(id, peerSide)) {
        qWarning() << "Received OpenChannel with invalid channel_identifier:" << QString::fromStdString(message.DebugString());
        // Deliberately invalid behavior; kill the connection
        closeChannel();
        return;
    }

    Data::Control::ChannelResult *response = new Data::Control::ChannelResult;
    response->set_channel_identifier(id);

    Channel *channel = Channel::create(QString::fromStdString(message.channel_type()), Inbound, connection());
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
            // The channel may think it's open, so force it to close
            channel->closeChannel();
        } else if (!connection()->d->insertChannel(channel)) {
            Q_ASSERT_X(false, "handleOpenChannel", "Valid channel refused by connection");
            qWarning() << "BUG: Valid channel refused by connection";
            response->set_opened(false);
            channel->closeChannel();
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

    if (response->opened())
        emit connection()->channelOpened(channel);
}

void ControlChannel::handleChannelResult(const Data::Control::ChannelResult &message)
{
    int id = message.channel_identifier();
    Channel *channel = connection()->channel(id);
    if (!channel) {
        qWarning() << "Received ChannelResult for unknown identifier, ignoring:" << QString::fromStdString(message.DebugString());
        return;
    }

    if (channel->direction() != Outbound || channel->isOpened()) {
        qWarning() << "Received (duplicate?) ChannelResult for existing channel in an unexpected state:" << QString::fromStdString(message.DebugString());
        return;
    }

    bool opened = channel->d_ptr->openChannelResult(&message);

    if (opened && !channel->isOpened()) {
        BUG() << "Outbound channel isn't open after successful ChannelResult";
        channel->closeChannel();
    } else if (!opened && channel->isOpened()) {
        BUG() << "Outbound channel is open after failed ChannelResult";
        channel->closeChannel();
    }

    // Channel::outboundOpenResult will invalidate on failure, causing the
    // instance to be deleted once it's safe to do so
    if (!opened || !channel->isOpened()) {
        if (connection()->channel(channel->identifier())) {
            BUG() << "Channel not invalidated after failed outbound OpenChannel request";
            channel->closeChannel();
        }
    } else {
        emit connection()->channelOpened(channel);
    }
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

void ControlChannel::handleEnableFeatures(const Data::Control::EnableFeatures &message)
{
    Q_UNUSED(message);
    // This version does not support any features.
    Data::Control::Packet responseMessage;
    responseMessage.mutable_features_enabled();
    sendMessage(responseMessage);
}

void ControlChannel::handleFeaturesEnabled(const Data::Control::FeaturesEnabled &message)
{
    Q_UNUSED(message);
    // This version does not generate EnableFeatures messages, so receiving this is an error.
    qDebug() << "Unexpectedly received FeaturesEnabled message from peer, but we never send EnableFeatures";
    closeChannel();
}

