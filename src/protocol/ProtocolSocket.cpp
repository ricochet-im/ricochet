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

#include "ProtocolSocket.h"
#include "ProtocolCommand.h"
#include "CommandHandler.h"
#include "IncomingSocket.h"
#include "tor/TorControl.h"
#include <QNetworkProxy>
#include <QtEndian>
#include <QDebug>

/* Create a new outgoing connection */
ProtocolSocket::ProtocolSocket(ContactUser *user)
    : QObject(user)
    , user(user)
    , m_socket(0)
    , nextCommandId(0)
{
    qRegisterMetaType<QAbstractSocket::SocketError>();
}

void ProtocolSocket::setSocket(QTcpSocket *socket)
{
    if (socket && socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "BUG: ProtocolSocket::setSocket with unconnected socket";
        socket = 0;
    }

    if (socket == m_socket)
        return;

    bool wasConnected = isConnected();

    if (m_socket) {
        /* The existing socket is replaced, and all pending commands
         * are considered failed. This could be improved on. */
        QTcpSocket *oldSocket = m_socket;
        m_socket = 0;

        oldSocket->disconnect(this);
        // XXX can this be avoided if none are sent yet?
        abortCommands();
        oldSocket->abort();
        oldSocket->deleteLater();
    }

    m_socket = socket;

    if (socket) {
        socket->setParent(this);
        connect(socket, SIGNAL(readyRead()), this, SLOT(read()));
        // QueuedConnection used to make sure socket states are updated first
        connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()),
                Qt::QueuedConnection);
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketDisconnected()), Qt::QueuedConnection);

        if (!wasConnected) {
            m_connectedTime.restart();
            emit connected();
        }

        flushCommands();
        read();
    } else {
        emit disconnected();
    }

    emit socketChanged();
}

bool ProtocolSocket::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

int ProtocolSocket::connectedDuration() const
{
    if (!isConnected())
        return 0;
    return int(m_connectedTime.elapsed() / 1000);
}

void ProtocolSocket::disconnect()
{
    setSocket(0);
}

#if 0
void ProtocolSocket::sendAuth()
{
    Q_ASSERT(!authPending && !authFinished);

    QByteArray intro = IncomingSocket::introData(ProtocolSocket::PurposePrimary);

    QByteArray secret = manager->secret();
    Q_ASSERT(secret.size() == 16);

    if (secret.size() != 16)
    {
        int f = secret.size();
        secret.resize(16);
        memset(secret.data() + f, 0, 16 - f);
    }

    intro.append(secret);

    qint64 re = socket->write(intro);
    Q_ASSERT(re == intro.size());

    authPending = true;
}
#endif

void ProtocolSocket::flushCommands()
{
    if (!m_socket)
        return;

    while (!commandQueue.isEmpty())
        m_socket->write(commandQueue.takeFirst()->commandBuffer);
}

quint16 ProtocolSocket::getIdentifier()
{
    if (!nextCommandId)
    {
        Q_ASSERT(pendingCommands.isEmpty());

        /* Start with a random ID; technically not necessary, but still worth doing */
        nextCommandId = (qrand() % 65535) + 1;
    }

    /* Wraparound on unsigned integer overflow is well-defined, and we rely
     * on it here. 0 is not a valid ID. */

    while (pendingCommands.contains(nextCommandId) || !nextCommandId)
        ++nextCommandId;

    quint16 re = nextCommandId;

    if (!++nextCommandId)
        nextCommandId++;

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

    Q_ASSERT(commandQueue.isEmpty());
    qint64 re = m_socket->write(command->commandBuffer);
    Q_ASSERT(re == command->commandBuffer.size());

    qDebug() << "Wrote command:" << command->commandBuffer.toHex();
}

void ProtocolSocket::read()
{
    qDebug() << "Socket readable";

    qint64 available;
    while ((available = m_socket->bytesAvailable()) >= 6)
    {
        quint16 msgLength;
        if (m_socket->peek(reinterpret_cast<char*>(&msgLength), sizeof(msgLength)) < 2)
            return;

        msgLength = qFromBigEndian(msgLength);
        if (!msgLength)
        {
            /* Unbuffered replies aren't implemented; the connection cannot continue */
            qWarning() << "Closing connection: Unbuffered protocol replies are not implemented";
            m_socket->close();
            return;
        }

        /* Message length is one more than the actual data length, and does not include the header. */
        msgLength--;
        if ((available - 6) < msgLength)
            break;

        QByteArray data;
        data.resize(msgLength + 6);

        qint64 re = m_socket->read(data.data(), msgLength + 6);
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
                    /* Duplicated in socketDisconnected() */
                    qDebug() << "Received final reply for identifier" << identifier;
                    emit (*it)->commandFinished();
                    (*it)->deleteLater();
                    pendingCommands.erase(it);
                }
            }
        }
        else
        {
            CommandHandler handler(user, m_socket, reinterpret_cast<const uchar*>(data.constData()),
                                   msgLength + 6);
        }
    }
}

void ProtocolSocket::socketDisconnected()
{
    if (!m_socket)
        return;

    setSocket(0);
}

void ProtocolSocket::abortCommands()
{
    /* Send failure replies for all pending commands */
    for (QHash<quint16,ProtocolCommand*>::Iterator it = pendingCommands.begin(); it != pendingCommands.end(); ++it)
    {
        (*it)->processReply(ProtocolCommand::ConnectionError, 0, 0);
        emit (*it)->commandFinished();
        (*it)->deleteLater();
    }

    for (QQueue<ProtocolCommand*>::Iterator it = commandQueue.begin(); it != commandQueue.end(); ++it)
    {
        (*it)->processReply(ProtocolCommand::ConnectionError, 0, 0);
        emit (*it)->commandFinished();
        (*it)->deleteLater();
    }

    pendingCommands.clear();
    commandQueue.clear();
}
