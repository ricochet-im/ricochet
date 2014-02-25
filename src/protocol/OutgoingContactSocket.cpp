/* Torsion - http://torsionim.org/
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

#include "OutgoingContactSocket.h"
#include "IncomingSocket.h"

OutgoingContactSocket::OutgoingContactSocket(QObject *parent)
    : QObject(parent)
    , m_socket(0)
    , m_purpose(Protocol::PurposePrimary)
{
    m_authTimeout.setInterval(10000);
    m_authTimeout.setSingleShot(true);
    connect(&m_authTimeout, SIGNAL(timeout()), SLOT(onTimeout()));
}

void OutgoingContactSocket::setAuthentication(Protocol::Purpose purpose, const QByteArray &secret)
{
    m_purpose = purpose;
    m_secret = secret;
}

void OutgoingContactSocket::connectToHost(const QString &hostname, quint16 port)
{
    if (m_socket && m_socket->hostName() == hostname && m_socket->port() == port)
        return;
    else if (m_socket)
        disconnect();

    m_socket = new Tor::TorSocket(this);
    connect(m_socket, SIGNAL(connected()), SLOT(onConnected()));
    connect(m_socket, SIGNAL(readyRead()), SLOT(onReadable()));
    m_socket->connectToHost(hostname, port);
}

void OutgoingContactSocket::disconnect()
{
    if (m_socket) {
        m_socket->disconnect(this);
        m_socket->abort();
        m_socket->deleteLater();
        m_socket = 0;
    }
    m_authTimeout.stop();
}

void OutgoingContactSocket::onConnected()
{
    QByteArray intro = IncomingSocket::introData(m_purpose);
    if (m_secret.size() < 16) {
        emit authenticationFailed();
        disconnect();
    }
    intro.append(m_secret);

    m_socket->write(intro);
    m_authTimeout.start();
}

void OutgoingContactSocket::onDisconnected()
{
    // TorSocket handles reconnection attempts
    m_authTimeout.stop();
}

void OutgoingContactSocket::onReadable()
{
    if (m_socket->bytesAvailable() < 2)
        return;

    char reply[2];
    qint64 re = m_socket->read(reply, 2);
    if (re != 2) {
        qDebug() << "Outgoing socket failed";
        // Close socket and retry automatically
        m_socket->close();
        return;
    }

    if (reply[0] != Protocol::ProtocolVersion) {
        qDebug() << "Outgoing socket rejected: Version negotiation failure";
        disconnect();
        emit versionNegotiationFailed();
        return;
    }

    if (reply[1] != 0x00) {
        qDebug() << "Outgoing socket rejected: Authentication failure, code" << hex << (int)reply[1];
        disconnect();
        emit authenticationFailed();
        return;
    }

    // Detach from socket and pass it on
    m_socket->disconnect(this);
    m_socket->setReconnectEnabled(false);
    m_authTimeout.stop();

    emit socketReady(m_socket);
    Q_ASSERT(m_socket->parent() != this);
    if (m_socket->parent() == this) {
        qWarning() << "Destroying new socket because it wasn't claimed by any handler";
        disconnect();
    }
}

void OutgoingContactSocket::onTimeout()
{
    if (!m_socket)
        return;

    qDebug() << "Authentication timed out on outgoing socket";
    // Will reconnect automatically after a delay
    m_socket->close();
}

