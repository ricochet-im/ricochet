/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#include "IncomingSocket.h"
#include "core/UserIdentity.h"
#include "ProtocolManager.h"
#include "core/ContactsManager.h"
#include "ContactRequestServer.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
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
        connect(conn, SIGNAL(readyRead()), this, SLOT(readSocket()));
        connect(conn, SIGNAL(disconnected()), this, SLOT(removeSocket()));

        conn->setParent(this);
        conn->setProperty("startTime", QDateTime::currentDateTime());
        pendingSockets.append(conn);

        if (!expireTimer.isActive())
            expireTimer.start(10000, this);
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
    QDateTime now = QDateTime::currentDateTime();

    for (int i = 0; i < pendingSockets.size(); ++i)
    {
        QDateTime started = pendingSockets[i]->property("startTime").toDateTime();
        if (started.secsTo(now) >= 30)
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
        if ((uchar)versions[i] == protocolVersion)
        {
            version = protocolVersion;
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
    Q_ASSERT(version == protocolVersion);

    /* Peek at the purpose; can't be a read as this may be called repeatedly until it's ready */
    uchar purpose;
    if (socket->peek(reinterpret_cast<char*>(&purpose), 1) < 1)
        return;

    /* Purposes less than 0x20 are allowed as contact-authenticated connections,
     * all following the same auth rules (with nothing else in intro).
     * 0x00 has special meaning as a primary connection, while all others are
     * treated as generic unidirectional auxiliary connections. */

    if (purpose < 0x20)
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
        user->conn()->addSocket(socket, purpose);
        Q_ASSERT(socket->parent() != this);
    }
    else if (purpose == 0x80)
    {
        /* Incoming contact request connection */

        /* Read purpose */
        int ok = socket->read(1).size();
        Q_ASSERT(ok);

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

QByteArray IncomingSocket::introData(uchar purpose)
{
    QByteArray re;
    re.resize(5);

    re[0] = 0x49;
    re[1] = 0x4D;
    re[2] = 0x01; /* number of versions */
    re[3] = protocolVersion; /* version */
    re[4] = (char)purpose;

    return re;
}
