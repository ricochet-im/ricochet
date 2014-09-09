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

#include "IncomingSocket.h"
#include "core/UserIdentity.h"
#include "core/ContactsManager.h"
#include "ContactRequestServer.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QElapsedTimer>
#include <QtDebug>

IncomingSocket::IncomingSocket(UserIdentity *id, QObject *parent)
    : QObject(parent), identity(id), server(new QTcpServer(this))
{
    connect(server, SIGNAL(newConnection()), this, SLOT(incomingConnection()));
}

bool IncomingSocket::listen(const QHostAddress &address, quint16 port)
{
    if (server->isListening())
        server->close();

    return server->listen(address, port);
}

QString IncomingSocket::errorString() const
{
    return server->errorString();
}

QHostAddress IncomingSocket::serverAddress() const
{
    return server->serverAddress();
}

quint16 IncomingSocket::serverPort() const
{
    return server->serverPort();
}

void IncomingSocket::incomingConnection()
{
    while (server->hasPendingConnections())
    {
        QTcpSocket *conn = server->nextPendingConnection();
        conn->setParent(this);
        connect(conn, SIGNAL(readyRead()), this, SLOT(readSocket()));
        connect(conn, SIGNAL(disconnected()), this, SLOT(removeSocket()));

        QElapsedTimer time;
        time.start();
        conn->setProperty("startTime", time.msecsSinceReference());
        pendingSockets.append(conn);

        if (!expireTimer.isActive())
            expireTimer.start(11000, this);
    }
}

void IncomingSocket::removeSocket(QTcpSocket *socket)
{
    if (!socket)
    {
        socket = qobject_cast<QTcpSocket*>(sender());
        if (!socket)
            return;
    }

    qDebug() << "Disconnecting pending socket";

    pendingSockets.removeOne(socket);

    socket->disconnect(this);
    socket->close();
    socket->deleteLater();
}

void IncomingSocket::timerEvent(QTimerEvent *)
{
    QElapsedTimer now;
    now.start();

    for (int i = 0; i < pendingSockets.size(); ++i)
    {
        qint64 started = pendingSockets[i]->property("startTime").toLongLong();
        if (now.msecsSinceReference() - started >= 10000)
        {
            /* time is up. */
            removeSocket(pendingSockets[i]);
            --i;
        }
    }

    if (pendingSockets.isEmpty())
        expireTimer.stop();
}

void IncomingSocket::readSocket()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket)
        return;

    QVariant versionProp = socket->property("protocolVersion");

    if (versionProp.isNull())
    {
        if (!handleVersion(socket))
            return;
        versionProp = socket->property("protocolVersion");

        if (versionProp.isNull())
        {
            Q_ASSERT_X(false, "IncomingConnection::handleVersion", "version property not properly set");
            return;
        }
    }

    handleIntro(socket, (uchar)versionProp.toUInt());
}

bool IncomingSocket::handleVersion(QTcpSocket *socket)
{
    /* 0x49 0x4D [numversions] (numversions * version) */
    qint64 available = socket->bytesAvailable();

    if (available < 3)
        return false;

    /* Peek for the identifier and version count */
    uchar intro[3];
    qint64 re = socket->peek(reinterpret_cast<char*>(intro), 3);
    if (re < 3)
        return false;

    if (intro[0] != 0x49 || intro[1] != 0x4D || intro[2] == 0)
    {
        qDebug() << "Connection rejected: incorrect introduction sequence";
        removeSocket(socket);
        return false;
    }

    /* Stop and wait if the full list of supported versions is not here */
    if (available < (intro[2] + 3))
        return false;

    QByteArray versions = socket->read(intro[2] + 3);
    Q_ASSERT(versions.size() == intro[2] + 3);

    /* Only one version is supported right now (protocolVersion). 0xff is the reserved failure code. */
    uchar version = 0xff;
    for (int i = 3; i < versions.size(); ++i)
    {
        if ((uchar)versions[i] == Protocol::ProtocolVersion)
        {
            version = Protocol::ProtocolVersion;
            break;
        }
    }

    /* Send the version response */
    socket->write(reinterpret_cast<char*>(&version), 1);

    if (version == 0xff)
    {
        qDebug() << "Connection rejected: no mutually supported protocol version";
        removeSocket(socket);
        return false;
    }

    /* Set the version property (used for the rest of the intro) */
    socket->setProperty("protocolVersion", QVariant((unsigned)version));

    return true;
}

void IncomingSocket::handleIntro(QTcpSocket *socket, uchar version)
{
    Q_ASSERT(version == Protocol::ProtocolVersion);
    Q_UNUSED(version);

    /* Peek at the purpose; can't be a read as this may be called repeatedly until it's ready */
    uchar purpose;
    if (socket->peek(reinterpret_cast<char*>(&purpose), 1) < 1)
        return;

    if (purpose == Protocol::PurposePrimary)
    {
        /* Wait until the auth data is available */
        quint64 available = socket->bytesAvailable();
        if (available < 16)
            return;

        QByteArray secret = socket->read(17);
        /* Remove purpose */
        secret.remove(0, 1);
        Q_ASSERT(secret.size() == 16);

        ContactUser *user = identity->contacts.lookupSecret(secret);

        /* Response; 0x00 is success, while all others are error. 0x01 is generic error. */
        char response = 0x01;

        if (!user)
        {
            qDebug() << "Connection authentication failed: no match for secret";
            response = 0x02;
            socket->write(&response, 1);
            removeSocket(socket);
            return;
        }

        qDebug() << "Connection authentication successful for contact" << user->uniqueID
                << "purpose" << hex << purpose;

        /* 0x00 is success */
        response = 0x00;
        socket->write(&response, 1);

        pendingSockets.removeOne(socket);
        socket->disconnect(this);

        /* The protocolmanager also takes ownership */
#ifndef PROTOCOL_NEW
        user->incomingProtocolSocket(socket);
#endif
        Q_ASSERT(socket->parent() != this);
    }
    else if (purpose == Protocol::PurposeContactReq)
    {
        /* Incoming contact request connection */

        /* Read purpose */
        int ok = socket->read(1).size();
        Q_ASSERT(ok);
        Q_UNUSED(ok);

        /* Pass to ContactRequestServer */
        pendingSockets.removeOne(socket);
        socket->disconnect(this);

        new ContactRequestServer(identity, socket);
        Q_ASSERT(socket->parent() != this);
    }
    else
    {
        /* Purpose unknown and not supported; just close the connection, because somebody
         * isn't obeying the rules of protocol versioning. */
        qDebug() << "Connection rejected: Unrecognized version 0 purpose" << hex << purpose;
        removeSocket(socket);
    }
}

QByteArray IncomingSocket::introData(Protocol::Purpose purpose)
{
    QByteArray re;
    re.resize(5);

    re[0] = 0x49;
    re[1] = 0x4D;
    re[2] = 0x01; /* number of versions */
    re[3] = Protocol::ProtocolVersion; /* version */
    re[4] = (char)purpose;

    return re;
}
