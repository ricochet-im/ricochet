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

#include "ContactRequestChannel.h"
#include "Channel_p.h"

using namespace Protocol;

/* Regarding message and nickname limitations:
 *
 * For messages, we should use limits the same as those of chat, including limits on the
 * length and content.
 *
 * For nicknames, we should be quite restrictive, and these should be applied consistently
 * with protocol and UI. It should be similar to the restrictions on filenames, particularly
 * by excluding control characters. Length limit is short.
 *
 * If the nickname duplicates an existing contact, it must be changed for the request, but
 * the peer must not be aware of this in any way.
 */

ContactRequestChannel::ContactRequestChannel(Direction direction, Connection *connection)
    : Channel(QStringLiteral("im.ricochet.contact.request"), direction, connection)
    , m_responseStatus(Data::ContactRequest::Response::Undefined)
{
}

QString ContactRequestChannel::message() const
{
    return m_message;
}

void ContactRequestChannel::setMessage(const QString &message)
{
    if (direction() != Outbound) {
        BUG() << "Request messages can only be set on outbound messages";
        return;
    }

    // Only valid before channel opened
    if (isOpened() || identifier() >= 0) {
        BUG() << "Request data must be set before opening channel";
        return;
    }

    if (message.size() > Data::ContactRequest::MessageMaxCharacters) {
        BUG() << "Outbound contact request message is too long (" << message.size() << ")";
        return;
    }

    m_message = message;
}

static bool isAcceptableNickname(const QString &input)
{
    if (input.size() > Data::ContactRequest::NicknameMaxCharacters)
        return false;

    /* Although nicknames should always be escaped before being displayed
     * in a HTML-sensitive context, there's little value in allowing these
     * characters in nicknames, and it could prevent future bugs. */
    QVector<uint> blacklist = QStringLiteral("\"<>&").toUcs4();
    QVector<uint> chars = input.toUcs4();
    foreach (uint value, chars) {
        QChar c(value);
        if (c.category() == QChar::Other_Format ||
            c.category() == QChar::Other_Control ||
            c.isNonCharacter() ||
            blacklist.contains(value))
            return false;
    }

    return true;
}

QString ContactRequestChannel::nickname() const
{
    return m_nickname;
}

void ContactRequestChannel::setNickname(const QString &nickname)
{
    if (direction() != Outbound) {
        BUG() << "Request messages can only be set on outbound messages";
        return;
    }

    if (isOpened() || identifier() >= 0) {
        BUG() << "Request data must be set before opening channel";
        return;
    }

    if (!isAcceptableNickname(nickname)) {
        BUG() << "Outbound contact request nickname isn't acceptable:" << nickname;
        return;
    }

    m_nickname = nickname;
}

bool ContactRequestChannel::allowInboundChannelRequest(const Data::Control::OpenChannel *request, Data::Control::ChannelResult *result)
{
    using namespace Data::ContactRequest;
    using namespace Data::Control;

    // If this connection is already KnownContact, report that the request is accepted
    if (connection()->purpose() == Connection::Purpose::KnownContact) {
        QScopedPointer<Response> response(new Response);
        response->set_status(Response::Accepted);
        result->SetAllocatedExtension(Data::ContactRequest::response, response.take());
        return false;
    }

    // We'll only accept requests on inbound connections with an unknown purpose
    if (connection()->direction() != Connection::ServerSide ||
        connection()->purpose() != Connection::Purpose::Unknown)
    {
        result->set_common_error(ChannelResult::BadUsageError);
        return false;
    }

    // Only allow one ContactRequestChannel
    if (connection()->findChannel<ContactRequestChannel>()) {
        result->set_common_error(ChannelResult::BadUsageError);
        return false;
    }

    // Require HiddenServiceAuth
    if (!connection()->hasAuthenticated(Connection::HiddenServiceAuth)) {
        result->set_common_error(ChannelResult::UnauthorizedError);
        return false;
    }

    if (!request->HasExtension(Data::ContactRequest::contact_request)) {
        result->set_common_error(ChannelResult::BadUsageError);
        return false;
    }

    ContactRequest contactData = request->GetExtension(Data::ContactRequest::contact_request);
    QString nickname = QString::fromStdString(contactData.nickname());
    QString message = QString::fromStdString(contactData.message_text());

    m_responseStatus = Response::Undefined;
    if (message.size() > Data::ContactRequest::MessageMaxCharacters ||
        !isAcceptableNickname(nickname))
    {
        qWarning() << "Rejecting incoming contact request with invalid nickname/message";
        setResponseStatus(Response::Error);
    } else {
        m_nickname = nickname;
        m_message = message;
        emit requestReceived();

        if (m_responseStatus == Response::Undefined) {
            BUG() << "No response to incoming contact request after requestReceived signal";
            setResponseStatus(Response::Error);
        }
    }

    QScopedPointer<Response> response(new Response);
    response->set_status(m_responseStatus);
    result->SetAllocatedExtension(Data::ContactRequest::response, response.take());

    // If the response is final, close the channel immediately once it's fully open
    if (m_responseStatus > Response::Pending)
        connect(this, &Channel::channelOpened, this, &Channel::closeChannel, Qt::QueuedConnection);
    return true;
}

void ContactRequestChannel::setResponseStatus(Status status)
{
    if (m_responseStatus == status)
        return;

    if (direction() != Inbound) {
        BUG() << "Can't set the response on an outbound contact request";
        return;
    }

    using namespace Data::ContactRequest;
    if (m_responseStatus > Response::Pending)
        BUG() << "Response status is already a final state" << m_responseStatus << "but was changed to" << status;

    m_responseStatus = status;

    // If the channel is already open, the response is sent as a separate packet
    if (isOpened()) {
        Response response;
        response.set_status(m_responseStatus);
        sendMessage(response);

        if (m_responseStatus > Response::Pending)
            closeChannel();
    }
}

bool ContactRequestChannel::allowOutboundChannelRequest(Data::Control::OpenChannel *request)
{
    if (connection()->direction() != Connection::ClientSide ||
        connection()->purpose() != Connection::Purpose::OutboundRequest)
    {
        BUG() << "ContactRequestChannel can only be used on OutboundRequest connections. Has purpose"
              << int(connection()->purpose());
        return false;
    }

    if (connection()->findChannel<ContactRequestChannel>()) {
        BUG() << "ContactRequestChannel can only be used once per connection";
        return false;
    }

    QScopedPointer<Data::ContactRequest::ContactRequest> contactData(new Data::ContactRequest::ContactRequest);
    if (!m_nickname.isEmpty())
        contactData->set_nickname(m_nickname.toStdString());
    if (!m_message.isEmpty())
        contactData->set_message_text(m_message.toStdString());

    request->SetAllocatedExtension(Data::ContactRequest::contact_request, contactData.take());
    return true;
}

bool ContactRequestChannel::processChannelOpenResult(const Data::Control::ChannelResult *result)
{
    if (!result->HasExtension(Data::ContactRequest::response)) {
        qDebug() << "Expected a response for the contact request";
        return false;
    }

    Data::ContactRequest::Response response = result->GetExtension(Data::ContactRequest::response);
    return handleResponse(&response);
}

void ContactRequestChannel::receivePacket(const QByteArray &packet)
{
    Data::ContactRequest::Response response;

    if (!response.ParseFromArray(packet.constData(), packet.size())) {
        qDebug() << "Invalid message received on contact request channel";
        closeChannel();
        return;
    }

    logger::println("receive {}\n{}", typeid(response), response.DebugString());

    if (!handleResponse(&response))
        closeChannel();
}

bool ContactRequestChannel::handleResponse(const Data::ContactRequest::Response *response)
{
    using namespace Data::ContactRequest;
    if (response->status() == Response::Undefined) {
        qDebug() << "Got an invalid response (undefined status) to a contact request";
        return false;
    }

    if (m_responseStatus > Response::Pending) {
        qDebug() << "Received a response" << response->status() << "to a contact request which already had a final response" << m_responseStatus;
        return false;
    }

    m_responseStatus = response->status();
    emit requestStatusChanged(m_responseStatus);
    // If the response is final, close the channel. Use a queued invoke to avoid any potential
    // issue when called from processChannelOpenResult
    if (m_responseStatus > Response::Pending)
        metaObject()->invokeMethod(this, "closeChannel", Qt::QueuedConnection);

    return true;
}

