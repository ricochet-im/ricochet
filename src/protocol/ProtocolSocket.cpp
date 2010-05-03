/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
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
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

#include "ProtocolSocket.h"
#include "ProtocolManager.h"
#include "ProtocolCommand.h"
#include "CommandHandler.h"
#include "tor/TorControlManager.h"
#include <QNetworkProxy>
#include <QtEndian>

/* Create with an established, authenticated connection */
ProtocolSocket::ProtocolSocket(QTcpSocket *s, ProtocolManager *m)
    : QObject(m), manager(m), socket(s), active(true), authPending(false), authFinished(true)
{
    Q_ASSERT(isConnected());

    socket->setParent(this);
    setupSocket();
}

/* Create a new outgoing connection */
ProtocolSocket::ProtocolSocket(ProtocolManager *m)
    : QObject(m), manager(m), socket(new QTcpSocket(this)), active(false), authPending(false), authFinished(false)
{
    connect(socket, SIGNAL(connected()), this, SLOT(sendAuth()));
    connect(this, SIGNAL(socketReady()), this, SLOT(flushCommands()));
    setupSocket();
}

void ProtocolSocket::setupSocket()
{
    connect(socket, SIGNAL(readyRead()), this, SLOT(read()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketDisconnected()));
}

bool ProtocolSocket::isConnected() const
{
    return (authFinished && socket->state() == QAbstractSocket::ConnectedState);
}

bool ProtocolSocket::isConnecting() const
{
    return (socket->state() != QAbstractSocket::UnconnectedState && socket->state() != QAbstractSocket::ClosingState
            && !isConnected());
}

void ProtocolSocket::connectToHost(const QString &host, quint16 port)
{
    if (isConnected() && socket->peerName() == host && socket->peerPort() == port)
        return;

    socket->abort();
    active = true;

    if (!torManager->isSocksReady())
    {
        /* Can't make any connection; behave as if the connection attempt failed.
         * Many things should handle this situation prior to this function anyway. */
        socketDisconnected();
        return;
    }

    socket->setProxy(torManager->connectionProxy());
    socket->connectToHost(host, port);
}

void ProtocolSocket::abort()
{
    socket->abort();
}

void ProtocolSocket::abortConnectionAttempt()
{
    if (!isConnecting())
        return;

    /* We need to ensure that socketDisconnected is called once even if there wasn't technically a
     * connection yet (as it handles errors too). */

    socket->blockSignals(true);
    socket->abort();
    socket->blockSignals(false);

    socketDisconnected();
}

void ProtocolSocket::sendAuth()
{
    Q_ASSERT(!authPending && !authFinished);

    QByteArray secret = manager->secret();
    Q_ASSERT(secret.size() == 16);

    quint8 purpose = 0x00;

    /* Introduction; 0x49 0x4D [1*version] [16*cookie] [1*purpose] */
    QByteArray intro;
    intro.resize(20);

    intro[0] = 0x49;
    intro[1] = 0x4D;
    intro[2] = protocolVersion;
    intro[19] = purpose;

    memcpy(intro.data()+3, secret.constData(), qMin(16, secret.size()));
    if (secret.size() < 16)
        memset(intro.data()+3+secret.size(), 0, 16-secret.size());

    qint64 re = socket->write(intro);
    Q_ASSERT(re == intro.size());

    authPending = true;
}

void ProtocolSocket::flushCommands()
{
    while (!commandQueue.isEmpty())
        socket->write(commandQueue.takeFirst()->commandBuffer);
}

quint16 ProtocolSocket::getIdentifier() const
{
    /* There is a corner case for the very unlucky where the RNG will take a very long time
     * to find an available ID. This could be considered a bug.
     *
     * This should probably just be sequential and wrap.
     */

    if (pendingCommands.size() >= 50000)
        return 0;

    quint16 re;
    do
    {
        re = (qrand() % 65535) + 1;
    } while (pendingCommands.contains(re));

    return re;
}

void ProtocolSocket::sendCommand(ProtocolCommand *command)
{
    Q_ASSERT(!pendingCommands.contains(command->identifier()));

    pendingCommands.insert(command->identifier(), command);

    if (!isConnected())
    {
        qDebug() << "Added command to queue";
        commandQueue.append(command);
        return;
    }

    /* TODO Use the command queue even when connected, enough to allow tracking of sent status
     * without impacting performance (i.e. still optimal use of the buffer). */

    Q_ASSERT(commandQueue.isEmpty());
    qint64 re = socket->write(command->commandBuffer);
    Q_ASSERT(re == command->commandBuffer.size());

    qDebug() << "Wrote command:" << command->commandBuffer.toHex();
}

void ProtocolSocket::read()
{
    qDebug() << "Socket readable";

    qint64 available = socket->bytesAvailable();

    if (available && authPending && !authFinished)
    {
        char reply;
        qint64 re = socket->read(&reply, 1);
        Q_ASSERT(re);

        if (reply != 0x01)
        {
            socket->close();
            return;
        }

        authPending = false;
        authFinished = true;

        /* This will take care of flushing commands as well */
        emit socketReady();

        available--;
    }

    while (available >= 6)
    {
        quint16 msgLength;
        if (socket->peek(reinterpret_cast<char*>(&msgLength), sizeof(msgLength)) < 2)
            return;

        msgLength = qFromBigEndian(msgLength);
        if (!msgLength)
            qFatal("Unbuffered protocol replies are not implemented");

        /* Message length is one more than the actual data length, and does not include the header. */
        msgLength--;
        if ((available - 6) < msgLength)
            break;

        QByteArray data;
        data.resize(msgLength + 6);

        qint64 re = socket->read(data.data(), msgLength + 6);
        Q_ASSERT(re == msgLength + 6);

        if (isReply(data[3]))
        {
            quint16 identifier = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(data.constData())+4);
            QHash<quint16,ProtocolCommand*>::Iterator it = pendingCommands.find(identifier);

            if (it != pendingCommands.end())
            {
                (*it)->processReply(data[3], reinterpret_cast<const uchar*>(data.constData())+6, msgLength);
                if (isFinal(data[3]))
                {
                    qDebug() << "Received final reply for identifier" << identifier;
                    emit (*it)->commandFinished();
                    (*it)->deleteLater();
                    pendingCommands.erase(it);
                }
            }
        }
        else
        {
            CommandHandler handler(manager->user, socket, reinterpret_cast<const uchar*>(data.constData()),
                                   msgLength + 6);
        }

        available -= msgLength + 6;
    }
}

void ProtocolSocket::socketDisconnected()
{
    if (!active)
        return;

    active = false;

    if (authFinished && !isConnecting())
        emit disconnected();
    else
        emit connectFailed();

    authFinished = authPending = false;

    /* TODO: EEK. These need to be handled properly! */
    commandQueue.clear();
    pendingCommands.clear();
}
