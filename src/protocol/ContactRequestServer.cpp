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

#include "ContactRequestServer.h"
#include "CommandDataParser.h"
#include "utils/CryptoKey.h"
#include "utils/SecureRNG.h"
#include "core/UserIdentity.h"
#include "core/ContactsManager.h"
#include "core/IncomingRequestManager.h"
#include <QElapsedTimer>
#include <QTcpSocket>
#include <QtEndian>
#include <QDebug>

ContactRequestServer::ContactRequestServer(UserIdentity *id, QTcpSocket *s)
    : identity(id), socket(s), state(WaitRequest)
{
    socket->setParent(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadable()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    QElapsedTimer now;
    now.start();
    qint64 elapsed = socket->property("startTime").toLongLong();
    if (!elapsed)
        elapsed = 0;
    else
        elapsed = now.msecsSinceReference() - elapsed;

    timeout = new QTimer(this);
    timeout->setSingleShot(true);
    timeout->start(15000 - elapsed);
    connect(timeout, SIGNAL(timeout()), SLOT(close()));

    qDebug() << "Contact request connection created; sending cookie";
    sendCookie();
}

void ContactRequestServer::close()
{
    socket->close();
}

void ContactRequestServer::socketDisconnected()
{
    qDebug() << "Contact request connection closed";
    deleteLater();
}

void ContactRequestServer::sendCookie()
{
    cookie = SecureRNG::random(16);
    if (cookie.size() != 16) {
        close();
        return;
    }

    qint64 re = socket->write(cookie);
    Q_ASSERT(re == cookie.size());
    Q_UNUSED(re);

    state = WaitRequest;
}

/* Returns true if the connection is still open (dependant on the response) */
bool ContactRequestServer::sendResponse(uchar response)
{
    Q_ASSERT(state != SentResponse);

    qint64 re = socket->write(reinterpret_cast<const char*>(&response), 1);
    Q_ASSERT(re);
    Q_UNUSED(re);

    if (response != 0x00)
        state = SentResponse;

    if (response > 0x01)
    {
        /* Everything except acknowledge (0x00) and accept (0x01) is an error or rejection */
        socket->close();
        return false;
    }

    return true;
}

void ContactRequestServer::sendAccept(ContactUser *user)
{
    /* Send the accepted response; immediately after this, both ends will treat the connection
     * as their newly established command connection. */
    sendResponse(0x01);

    qDebug() << "Contact request accepted with an active connection; sending accept and morphing to command";

    socket->disconnect(this);
#ifndef PROTOCOL_NEW
    user->incomingProtocolSocket(socket);
#endif
    Q_ASSERT(socket->parent() != this);

    deleteLater();
}

void ContactRequestServer::sendRejection()
{
    sendResponse(0x40);
}

void ContactRequestServer::socketReadable()
{
    if (state != WaitRequest)
    {
        /* After the request has arrived, any data from the client is an error.
         * Drop the connection to prevent bandwidth consumption. */
        socket->close();
        return;
    }

    /* Peek at the request length field */
    quint16 length;
    if (socket->peek(reinterpret_cast<char*>(&length), sizeof(length)) < (int)sizeof(length))
        return;

    length = qFromBigEndian(length);
    if (socket->bytesAvailable() < length)
        return;

    QByteArray request = socket->read(length);
    if (request.size() < length) {
        // Shouldn't happen because of bytesAvailable; just panic and close.
        socket->close();
        return;
    }

    handleRequest(request);
}

void ContactRequestServer::handleRequest(const QByteArray &data)
{
    if (data.size() < 58) {
        // Impossibly small request
        qDebug() << "Incoming contact request is an impossibly small " << data.size() << "bytes; rejecting";
        sendResponse(0x80);
        return;
    }

    /* [2*length][16*hostname][16*serverCookie][16*connSecret][data:pubkey][str:nick][str:message][data:signature] */
    CommandDataParser request(&data);
    request.setPos(2);

    QByteArray hostname, receivedCookie, connSecret, encodedPublicKey, signature;
    QString nickname, message;

    request.readFixedData(&hostname, 16);
    request.readFixedData(&receivedCookie, 16);
    request.readFixedData(&connSecret, 16);
    request.readVariableData(&encodedPublicKey);
    request >> nickname >> message;
    int signaturePos = request.pos();
    request.readVariableData(&signature);

    if (request.hasError()) {
        qDebug() << "Incoming contact request syntax error; rejecting";
        sendResponse(0x80);
        return;
    }

    /* Verify serverHostname and serverCookie */
    if (hostname != identity->hostname().mid(0, 16).toLatin1() || receivedCookie != cookie) {
        qDebug() << "Incoming contact request has invalid hostname/cookie; rejecting";
        sendResponse(0x81);
        return;
    }

    /* Load the public key */
    CryptoKey key;
    if (!key.loadFromData(encodedPublicKey, CryptoKey::PublicKey)) {
        qDebug() << "Incoming contact request has an unparsable public key; rejecting";
        sendResponse(0x81);
        return;
    }

    /* Verify the signature */
    if (!key.verifyData(data.mid(2, signaturePos - 2), signature)) {
        qDebug() << "Incoming contact request has an invalid signature; rejecting";
        sendResponse(0x81);
        return;
    }

    /* Request is valid */
    QByteArray remoteHostname = key.torServiceID().toLatin1();
    qDebug() << "Received contact request:";
    qDebug() << "  Hostname:" << remoteHostname;
    qDebug() << "  Connection Secret:" << connSecret.toHex();
    qDebug() << "  Nickname:" << nickname;
    qDebug() << "  Message:" << message;
    qDebug() << "  Cookie:" << cookie.toHex();

    identity->contacts.incomingRequests.addRequest(remoteHostname, connSecret, this, nickname, message);

    /* addRequest() can automatically accept or reject in certain situations; account for that */
    if (state == SentResponse)
        return;

    /* Acknowledgement */
    sendResponse(0x00);

    /* We are now waiting for acceptance from the user; connection is held open. */
    timeout->stop();
    state = WaitResponse;
    return;
}

