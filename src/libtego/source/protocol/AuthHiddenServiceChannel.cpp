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

#include "AuthHiddenServiceChannel.h"
#include "AuthHiddenService.pb.h"
#include "Connection.h"
#include "Channel_p.h"
#include "utils/SecureRNG.h"
#include "utils/CryptoKey.h"
#include "utils/Useful.h"
#include "utils/StringUtil.h"
#include "error.hpp"

using namespace Protocol;

namespace Protocol {

constexpr size_t COOKIE_SIZE = 16;

class AuthHiddenServiceChannelPrivate : public ChannelPrivate
{
public:
    CryptoKey privateKey;
    QByteArray clientCookie, serverCookie;
    bool accepted;

    AuthHiddenServiceChannelPrivate(Channel *q, Channel::Direction dir, Connection *conn)
        : ChannelPrivate(q, QStringLiteral("im.ricochet.auth.hidden-service"), dir, conn)
        , accepted(false)
    {
    }

    QByteArray getProofData(const QByteArray& clientServiceId) const;
private:
    QByteArray getProofKey() const;
    QByteArray getProofMessage(const QByteArray &clientServiceId) const;
};

}

AuthHiddenServiceChannel::AuthHiddenServiceChannel(Direction dir, Connection *conn)
    : Channel(new AuthHiddenServiceChannelPrivate(this, dir, conn))
{
    if (direction() == Outbound)
        connect(this, &Channel::channelOpened, this, &AuthHiddenServiceChannel::sendAuthMessage);

    connect(this, &Channel::invalidated, this,
        [this]() {
            Q_D(AuthHiddenServiceChannel);
            if (d->accepted)
                emit authSuccessful();
            else
                emit authFailed();
        }
    );
}

void AuthHiddenServiceChannel::setPrivateKey(const CryptoKey &key)
{
    Q_D(AuthHiddenServiceChannel);
    if (isOpened()) {
        TEGO_BUG() << "Channel is already open";
        return;
    }

    if (!key.isLoaded() || !key.isPrivate()) {
        TEGO_BUG() << "AuthHiddenServiceChannel cannot authenticate without a valid private key";
        return;
    }

    d->privateKey = key;
}

bool AuthHiddenServiceChannel::allowInboundChannelRequest(const Data::Control::OpenChannel *request, Data::Control::ChannelResult *result)
{
    Q_D(AuthHiddenServiceChannel);

    using namespace Data::Control;

    if (connection()->direction() != Connection::ServerSide) {
        // Hidden service authentication is only allowed from the client-side connection
        qDebug() << "Rejecting AuthHiddenServiceChannel from server side";
        result->set_common_error(ChannelResult::BadUsageError);
        return false;
    }

    if (connection()->hasAuthenticated(Connection::HiddenServiceAuth)) {
        // You can only authenticate a connection once
        qDebug() << "Rejecting AuthHiddenServiceChannel on authenticated connection";
        result->set_common_error(ChannelResult::BadUsageError);
        return false;
    }

    if (connection()->findChannel<AuthHiddenServiceChannel>()) {
        // Refuse if another channel already exists
        qDebug() << "Rejecting instance of AuthHiddenServiceChannel on a connection that already has one";
        result->set_common_error(ChannelResult::BadUsageError);
        return false;
    }

    // Store client cookie
    std::string clientCookie = request->GetExtension(Data::AuthHiddenService::client_cookie);
    if (clientCookie.size() != COOKIE_SIZE) {
        qDebug() << "Received OpenChannel for" << type() << "with no valid client_cookie";
        result->set_common_error(ChannelResult::BadUsageError);
        return false;
    }
    TEGO_THROW_IF_FALSE(clientCookie.size() <= std::numeric_limits<int>::max());
    d->clientCookie = QByteArray(clientCookie.c_str(), static_cast<int>(clientCookie.size()));

    // Generate a random cookie and return result
    d->serverCookie = SecureRNG::random(COOKIE_SIZE);
    if (d->serverCookie.isEmpty())
        return false;

    qDebug() << "Accepted inbound AuthHiddenServiceChannel";

    result->SetExtension(Data::AuthHiddenService::server_cookie, std::string(d->serverCookie.constData(), static_cast<size_t>(d->serverCookie.size())));
    return true;
}

bool AuthHiddenServiceChannel::allowOutboundChannelRequest(Data::Control::OpenChannel *request)
{
    Q_D(AuthHiddenServiceChannel);

    if (!d->privateKey.isLoaded()) {
        TEGO_BUG() << "AuthHiddenServiceChannel can't be opened without a private key";
        return false;
    }

    d->clientCookie = SecureRNG::random(COOKIE_SIZE);
    if (d->clientCookie.isEmpty())
        return false;
    request->SetExtension(Data::AuthHiddenService::client_cookie, std::string(d->clientCookie.constData(), static_cast<size_t>(d->clientCookie.size())));
    return true;
}

bool AuthHiddenServiceChannel::processChannelOpenResult(const Data::Control::ChannelResult *result)
{
    Q_D(AuthHiddenServiceChannel);

    if (result->opened()) {
        std::string cookie = result->GetExtension(Data::AuthHiddenService::server_cookie);
        if (cookie.size() != COOKIE_SIZE) {
            qDebug() << "Received ChannelResult for" << type() << "with no valid server_cookie";
            return false;
        }
        
        TEGO_THROW_IF_FALSE(cookie.size() <= std::numeric_limits<int>::max());
        d->serverCookie = QByteArray(cookie.c_str(), static_cast<int>(cookie.size()));
        return true;
    }

    return false;
}

void AuthHiddenServiceChannel::sendAuthMessage()
{
    Q_D(AuthHiddenServiceChannel);

    if (direction() != Outbound) {
        TEGO_BUG() << "Proof message is only sent from outbound channels";
        return;
    }

    if (!isOpened())
        return;

    if (d->clientCookie.size() != COOKIE_SIZE || d->serverCookie.size() != COOKIE_SIZE) {
        TEGO_BUG() << "AuthHiddenServiceChannel can't create a proof without valid cookies";
        closeChannel();
        return;
    }

    QByteArray proofData = d->getProofData(d->privateKey.torServiceID());
    auto signature = d->privateKey.signData(proofData);

    QScopedPointer<Data::AuthHiddenService::Proof> proof(new Data::AuthHiddenService::Proof);
    proof->set_signature(std::string(signature.constData(), static_cast<size_t>(signature.size())));

    proof->set_service_id(d->privateKey.torServiceID().toStdString());

    Data::AuthHiddenService::Packet message;
    message.set_allocated_proof(proof.take());
    sendMessage(message);

    qDebug() << "AuthHiddenServiceChannel sent outbound authentication packet";
}

QByteArray AuthHiddenServiceChannelPrivate::getProofData(const QByteArray& clientServiceId) const
{
    auto proofMessage = this->getProofMessage(clientServiceId);
    auto proofKey = this->getProofKey();

    auto proofData = QMessageAuthenticationCode::hash(proofMessage, proofKey, QCryptographicHash::Sha256);
    return proofData;

}

QByteArray AuthHiddenServiceChannelPrivate::getProofKey() const
{
    if (clientCookie.size() != COOKIE_SIZE)
    {
        TEGO_BUG() << "Invalid client cookie size; should be " << COOKIE_SIZE;
        return {};
    }
    if (serverCookie.size() != COOKIE_SIZE)
    {
        TEGO_BUG() << "Invalid server cookie size; should be " << COOKIE_SIZE;
        return {};
    }

    return this->clientCookie + this->serverCookie;
}

QByteArray AuthHiddenServiceChannelPrivate::getProofMessage(const QByteArray& clientServiceId) const
{
    // TODO: prepenend string 'ricochet-refresh proof' to the message to prevent hash reuse
    QByteArray serverServiceId = connection->serverServiceId();

    if (clientServiceId.size() != TEGO_V3_ONION_SERVICE_ID_LENGTH || serverServiceId.size() != TEGO_V3_ONION_SERVICE_ID_LENGTH) {
        TEGO_BUG() << "AuthHiddenServiceChannel can't figure out the client and server hostnames";
        return {};
    }

    auto proofMessage = clientServiceId + serverServiceId;
    return proofMessage;
}

void AuthHiddenServiceChannel::receivePacket(const QByteArray &packet)
{
    Data::AuthHiddenService::Packet message;
    if (!message.ParseFromArray(packet.constData(), packet.size())) {
        closeChannel();
        return;
    }

    if (message.has_proof()) {
        handleProof(message.proof());
    } else if (message.has_result()) {
        handleResult(message.result());
    } else {
        qWarning() << "Unrecognized message on" << type();
        closeChannel();
    }
}

void AuthHiddenServiceChannel::handleProof(const Data::AuthHiddenService::Proof &message)
{
    Q_D(AuthHiddenServiceChannel);

    if (direction() != Inbound) {
        qWarning() << "Received unexpected proof on outbound" << type();
        closeChannel();
        return;
    }

    if (d->clientCookie.size() != COOKIE_SIZE || d->serverCookie.size() != COOKIE_SIZE) {
        TEGO_BUG() << "AuthHiddenServiceChannel can't create a proof without valid cookies";
        closeChannel();
        return;
    }

    TEGO_THROW_IF_FALSE(message.signature().size() <= std::numeric_limits<int>::max());
    TEGO_THROW_IF_FALSE(message.service_id().size() <= std::numeric_limits<int>::max());
    QByteArray signature(message.signature().c_str(), static_cast<int>(message.signature().size()));
    QByteArray serviceId(message.service_id().c_str(), static_cast<int>(message.service_id().size()));

    QScopedPointer<Data::AuthHiddenService::Result> result(new Data::AuthHiddenService::Result);

    result->set_accepted(false);

    if (signature.size() == TEGO_ED25519_SIGNATURE_SIZE)
    {
        CryptoKey publicKey;
        if(publicKey.loadFromServiceId(serviceId))
        {
            auto proofData = d->getProofData(serviceId);
            if (publicKey.verifyData(proofData, signature))
            {
                result->set_accepted(true);
            }
            else
            {
                qWarning() << "Signature verification failed on" << type();
            }
        }
        else
        {
            qWarning() << "Unable to parse public key from" << type();
        }
    }
    else
    {
        qWarning() << "Received Signature with incorrect size from" << type();
    }

    if (result->accepted())
    {
        // TODO: send back our own signature with our private key for server to verify
        const auto hostname = serviceId + ".onion";
        connection()->grantAuthentication(Connection::HiddenServiceAuth, hostname);
        d->accepted = true;
        result->set_is_known_contact(connection()->purpose() == Connection::Purpose::KnownContact);
    } else
    {
        d->accepted = false;
    }

    Data::AuthHiddenService::Packet resultMessage;
    resultMessage.set_allocated_result(result.data());
    sendMessage(resultMessage);

    // Clear QScopedPointer, value is now owned by the Packet
    result.take();

    // In all cases, close the channel afterwards. This also emits the
    // authSucceeded or authFailed signals.
    closeChannel();
}

void AuthHiddenServiceChannel::handleResult(const Data::AuthHiddenService::Result &message)
{
    Q_D(AuthHiddenServiceChannel);

    if (direction() != Outbound) {
        qWarning() << "Received invalid message on AuthHiddenServiceChannel";
        closeChannel();
        return;
    }

    if (message.accepted()) {
        qDebug() << "AuthHiddenServiceChannel succeeded as" << (message.is_known_contact() ? "known" : "unknown") << "contact";
        d->accepted = true;
        if (message.is_known_contact())
            connection()->grantAuthentication(Connection::KnownToPeer);
    } else {
        qWarning() << "AuthHiddenServiceChannel rejected";
        d->accepted = false;
    }

    closeChannel();
}

