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

#ifndef PROTOCOL_CHANNEL_H
#define PROTOCOL_CHANNEL_H

#include "ControlChannel.pb.h"

namespace Protocol
{

class Connection;
class ChannelPrivate;

/* Base representation of a channel inside of a connection
 *
 * Channel is subclassed by channel type implementations to handle channel
 * requests and inbound or outbound packets. Generally, the channel subclass
 * implements low-level communication, and a higher level class will wrap
 * the channel with real functionality.
 *
 * Outbound channels are opened by creating an instance of the channel type,
 * setting up any necessary properties, and sending it to
 * ControlChannel::openChannel. The result is reported through the
 * outboundOpenResult callback, which will emit channelOpened or channelRejected.
 *
 * Incoming channel requests create an instance by name via the create method
 * and call the inboundOpenChannel method. If that method indicates success, the
 * channel is inserted. If failed, the channel will be closed and destroyed.
 *
 * When a channel is closed, the instance is invalidated and will be deleted
 * automatically. Pointers to channel should be stored using QPointer, or should
 * be reset immediately when the invalidated() signal is emitted.
 */
class Channel : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Channel)
    Q_DECLARE_PRIVATE(Channel)

    friend class ControlChannel;
    friend class Connection;
    friend class ConnectionPrivate;

public:
    enum Direction {
        Invalid = -1,
        Inbound,
        Outbound
    };

    /* Create a Channel instance of the specified type
     *
     * Returns null if 'type' is unrecognized.
     */
    static Channel *create(const QString &type, Direction direction, Connection *connection);

    QString type() const;
    int identifier() const;
    Direction direction() const;
    Connection *connection();
    bool isOpened() const;

    /* Send the OpenChannel request for this channel
     *
     * Only valid when the channel hasn't been opened yet. If successful,
     * identifier() will be set and this function returns true. The channel
     * isn't open until the response arrives, signalled by the channelOpened
     * or channelRejected signals.
     *
     * If the channel is rejected, it will asynchronously emit the channelRejected
     * signal, and will be invalidated and deleted.
     *
     * If this function returns false, the request wasn't sent due to a local
     * error. In this case, the channel is also invalidated and will be deleted.
     */
    bool openChannel();

signals:
    void channelOpened();
    void channelRejected(Data::Control::ChannelResult::CommonError error);

    /* Emitted when the channel has become invalid and will be destroyed
     *
     * This signal is emitted when a channel is closed, an outbound channel request is
     * rejected, or the connection is lost. It indicates that the channel is no longer
     * valid and will be deleted once control reaches the event loop (i.e.
     * QObject::deleteLater).
     *
     * Any object using the channel must clear all references to it when this signal
     * is emitted.
     */
    void invalidated();

public slots:
    void closeChannel();

protected:
    explicit Channel(const QString &type, Direction direction, Connection *connection);
    explicit Channel(ChannelPrivate *d);
    virtual ~Channel();

    /* Determine the response to an inbound OpenChannel request
     *
     * Subclasses must implement this method to accept inbound OpenChannel requests.
     * The subclass implements any type-specific rules, and may update the result
     * object with error messages or other data.
     *
     * Basic sanity checking is performed before this method is called; it may
     * assume that the channel is in a sane state to receive an inbound request.
     *
     * The channel will be opened if this method returns true.
     */
    virtual bool allowInboundChannelRequest(const Data::Control::OpenChannel *request, Data::Control::ChannelResult *result) = 0;

    /* Determine whether to send an outbound OpenChannel request
     *
     * Subclasses may implement this method to approve outbound OpenChannel requests,
     * and attach type-specific data to the request.
     *
     * Basic sanity checking is performed before this method is called; it may
     * assume that the channel is in a sane state to send an outbound request.
     *
     * Return true to send the OpenChannel request, false to cancel.
     */
    virtual bool allowOutboundChannelRequest(Data::Control::OpenChannel *request) = 0;

    /* Process data from the response to an outbound OpenChannel request
     *
     * Subclasses may implement this method to handle data attached to a
     * ChannelResult message, received in response to an outbound OpenChannel
     * request. This method is called for all valid responses, including failure.
     *
     * Basic sanity checking is performed before this method is called; it may
     * assume that the channel was waiting for a response to an outbound request.
     *
     * Regardless of the result, the channel is not yet open when this method is
     * called. The channel will be opened (or failed and invalidated) afterwards.
     *
     * Return true to continue handling the response, false to cancel and close the
     * channel. The default implementation always returns true.
     */
    virtual bool processChannelOpenResult(const Data::Control::ChannelResult *result);

    /* Process data from an inbound packet for this channel
     *
     * Subclasses must implement this method to handle inbound packets for this
     * channel. 'packet' is raw data from the packet, and will not be empty.
     *
     * Generally, a channel will parse packets using the protobuf ParseFromArray
     * method of their packet message type, and call appropriate handlers for
     * the messages it contains.
     */
    virtual void receivePacket(const QByteArray &packet) = 0;

    /* Send raw data as a packet on this channel
     *
     * Sends the contents of 'packet' as a packet for this channel. Often, you
     * will not use this method directly, in favor of a method like sendMessage
     * that handles data serialization as well. 'packet' must not be empty.
     *
     * If this method returns false, the packet was not sent due to an error
     * with the state or contents of the packet. The caller is responsible for
     * handling any response to that failure.
     *
     * If this method returns true, but the packet later fails to send due to
     * a network issue, the channel will be closed.
     */
    bool sendPacket(const QByteArray &packet);

    /* Serialize a protobuf message and send it as a packet on this channel
     *
     * This function behaves like sendPacket, except that it accepts a
     * templated subclass of google::protobuf::Message, and serializes that
     * message into the packet. In addition to the cases where sendPacket
     * returns false, this function will return false if serialization fails.
     */
    template<typename T> bool sendMessage(const T &message);

    /* Get approval for an inbound channel from the Connection's handlers
     *
     * Channels that require approval from higher-layer functionality before
     * opening can use this method to emit the
     * Connection::channelRequestingInboundApproval signal. For example, this
     * can be used to look up whether an identifier for a channel is recognized,
     * and allow higher layers to attach signals and update data before the
     * channel is fully open.
     *
     * Approval should be signaled to the channel by means of some
     * channel-specific API; in many cases, that might involve setting certain
     * properties that the channel requires.
     *
     * This method may only be called from within the
     * allowInboundChannelRequest handler.
     */
    void requestInboundApproval();

    QScopedPointer<ChannelPrivate> d_ptr;
};

}

#endif
