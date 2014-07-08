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

#include "ContactRequestClient.h"
#include "core/ContactUser.h"
#include "core/UserIdentity.h"
#include "IncomingSocket.h"
#include "CommandDataParser.h"
#include "ProtocolConstants.h"
#include "tor/HiddenService.h"
#include "tor/TorSocket.h"
#include "utils/CryptoKey.h"
#include <QNetworkProxy>
#include <QtEndian>
#include <QTimer>
#include <QDebug>

ContactRequestClient::ContactRequestClient(ContactUser *u)
    : QObject(u), user(u), socket(0), m_response(NoResponse), state(NotConnected)
{
}

void ContactRequestClient::setMessage(const QString &message)
{
    m_message = message;
}

void ContactRequestClient::setMyNickname(const QString &nick)
{
    m_mynick = nick;
}

void ContactRequestClient::close()
{
    if (socket)
    {
        socket->disconnect(this);
        socket->abort();
        socket->deleteLater();
        socket = 0;
    }

    state = NotConnected;
}

void ContactRequestClient::sendRequest()
{
    close();

    socket = new Tor::TorSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadable()));
    socket->setMaxAttemptInterval(600);

    state = WaitConnect;
    socket->connectToHost(user->hostname(), user->port());
}

void ContactRequestClient::socketConnected()
{
    socket->write(IncomingSocket::introData(Protocol::PurposeContactReq));
    state = WaitConnect;

    qDebug() << "Contact request for" << user->uniqueID << "connected";
}

void ContactRequestClient::socketReadable()
{
    switch (state)
    {
    case WaitConnect:
        {
            uchar version;
            if (socket->read(reinterpret_cast<char*>(&version), 1) < 1)
                return;

            if (version != Protocol::ProtocolVersion)
            {
                emit rejected(Protocol::RequestVersionError);
                close();
                return;
            }

            state = WaitCookie;

            /* Deliberately omitted break; cookie may arrive instantly */
        }

    case WaitCookie:
        if (socket->bytesAvailable() < Protocol::RequestCookieSize)
            return;

        if (!buildRequestData(socket->read(Protocol::RequestCookieSize)))
        {
            socket->close();
            return;
        }

        state = WaitAck;
        break;

    case WaitAck:
    case WaitResponse:
        if (!handleResponse() && socket)
        {
            socket->close();
            return;
        }

        break;

    default:
        break;
    }
}

bool ContactRequestClient::buildRequestData(QByteArray cookie)
{
    /* [2*length][16*hostname][16*serverCookie][16*connSecret][data:pubkey][str:nick][str:message][data:signature] */
    QByteArray requestData;
    CommandDataParser request(&requestData);

    /* Hostname */
    QString hostname = user->hostname();
    hostname.truncate(hostname.lastIndexOf(QLatin1Char('.')));
    if (hostname.size() != 16)
    {
        qWarning() << "Cannot send contact request: unable to determine the remote service hostname";
        return false;
    }

    /* Connection secret */
    QByteArray connSecret = user->settings()->read<Base64Encode>("localSecret");
    if (connSecret.size() != 16)
    {
        qWarning() << "Cannot send contact request: invalid local secret";
        return false;
    }

    /* Public service key */
    Tor::HiddenService *service = user->identity->hiddenService();
    CryptoKey serviceKey;
    if (!service || !(serviceKey = service->cryptoKey()).isLoaded())
    {
        qWarning() << "Cannot send contact request: failed to load service key";
        return false;
    }

    QByteArray publicKeyData = serviceKey.encodedPublicKey();
    if (publicKeyData.isNull())
    {
        qWarning() << "Cannot send contact request: failed to encode service key";
        return false;
    }

    /* Build request */
    request << (quint16)0; /* placeholder for length */
    request.writeFixedData(hostname.toLatin1());
    request.writeFixedData(cookie);
    request.writeFixedData(connSecret);
    request.writeVariableData(publicKeyData);
    request << myNickname() << message();

    if (request.hasError())
    {
        qWarning() << "Cannot send contact request: command building failed";
        return false;
    }

    /* Sign request, excluding the length field */
    QByteArray signature = serviceKey.signData(requestData.mid(2));
    if (signature.isNull())
    {
        qWarning() << "Cannot send contact request: failed to sign request";
        return false;
    }

    request.writeVariableData(signature);
    if (request.hasError())
    {
        qWarning() << "Cannot send contact request: command building failed";
        return false;
    }

    /* Set length */
    qToBigEndian((quint16)requestData.size(), reinterpret_cast<uchar*>(requestData.data()));

    /* Send */
    qint64 re = socket->write(requestData);
    Q_ASSERT(re == requestData.size());
    Q_UNUSED(re);

    qDebug() << "Contact request for" << user->uniqueID << "sent request data";
    return true;
}

bool ContactRequestClient::handleResponse()
{
    uchar response;
    if (socket->read(reinterpret_cast<char*>(&response), 1) < 1)
        return true;

    /* TODO much more state handling and cleanup */

    switch (response)
    {
    case 0x00: /* Acknowledge */
        qDebug() << "Contact request for" << user->uniqueID << "acknowledged; waiting for response";
        state = WaitResponse;
        m_response = Acknowledged;
        emit acknowledged();
        break;

    case 0x01: /* Accept */
        qDebug() << "Contact request for" << user->uniqueID << "accepted! Converting connection to primary";

        m_response = Accepted;
        emit accepted();

        socket->disconnect(this);
        user->incomingProtocolSocket(socket);
        Q_ASSERT(socket->parent() != this);
        socket = 0;

        break;

    case 0x40:
        qDebug() << "Contact request for" << user->uniqueID << "rejected by user";
        m_response = Rejected;
        break;

    default: /* Error */
        qDebug() << "Contact request for" << user->uniqueID << "rejected with code" << hex << (int)response;
        m_response = Error;
        break;
    }

    emit responseChanged();

    if (m_response >= Rejected)
    {
        emit rejected(response);
        return false;
    }

    return true;
}
