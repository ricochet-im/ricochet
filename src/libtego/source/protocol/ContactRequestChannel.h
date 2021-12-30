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

#ifndef PROTOCOL_CONTACTREQUESTCHANNEL_H
#define PROTOCOL_CONTACTREQUESTCHANNEL_H

#include "Channel.h"
#include "ContactRequestChannel.pb.h"

namespace Protocol
{

class ContactRequestChannel : public Channel
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactRequestChannel)

public:
    typedef Data::ContactRequest::Response::Status Status;

    explicit ContactRequestChannel(Direction direction, Connection *connection);

    QString message() const;
    QString nickname() const;

    // Outbound
    void setMessage(const QString &message);
    void setNickname(const QString &nickname);

    // Inbound
    void setResponseStatus(Status status);

signals:
    /* Emitted during the inbound channel request handler, when a new request
     * arrives. A handler is expected to synchronously call setResponseStatus
     * if it claims this request; otherwise, it will be closed.
     */
    void requestReceived();
    void requestStatusChanged(Status status);

protected:
    virtual bool allowInboundChannelRequest(const Data::Control::OpenChannel *request, Data::Control::ChannelResult *result);
    virtual bool allowOutboundChannelRequest(Data::Control::OpenChannel *request);
    virtual bool processChannelOpenResult(const Data::Control::ChannelResult *result);
    virtual void receivePacket(const QByteArray &packet);

private:
    QString m_nickname;
    QString m_message;
    Status m_responseStatus;

    bool handleResponse(const Data::ContactRequest::Response *response);
};

}

#endif
