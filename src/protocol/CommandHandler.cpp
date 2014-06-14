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

#include "CommandHandler.h"
#include "ProtocolCommand.h"
#include "ProtocolConstants.h"
#include <QtEndian>
#include <QtDebug>
#include <QTcpSocket>

CommandHandler::CommandFunc CommandHandler::handlerMap[256] = { 0 };

CommandHandler::CommandHandler(ContactUser *u, QTcpSocket *s, const uchar *m, unsigned mS)
    : user(u),
      data((mS > Protocol::HeaderSize) ? QByteArray::fromRawData(reinterpret_cast<const char*>(m+Protocol::HeaderSize), mS-Protocol::HeaderSize) : QByteArray()),
      socket(s)
{
    Q_ASSERT(mS >= Protocol::HeaderSize);
    Q_ASSERT(Protocol::HeaderSize == 6);
    command = m[2];
    state = m[3];
    identifier = qFromBigEndian<quint16>(m+4);

    CommandFunc handler = handlerMap[command];

    qDebug() << "Handling command 0x" << hex << command << "state" << state << "from socket"
            << (void*)s << "with handler" << (void*)handler;

    if (!handler)
    {
        sendReply(Protocol::UnknownCommand);
        return;
    }

    handler(*this);
}

void CommandHandler::sendReply(quint8 state, const QByteArray &data)
{
    QByteArray message;
    message.reserve(data.size() + Protocol::HeaderSize);
    message.resize(Protocol::HeaderSize);

    qToBigEndian(quint16(data.size()), reinterpret_cast<uchar*>(message.data()));
    message[2] = command;
    message[3] = state;
    qToBigEndian(identifier, reinterpret_cast<uchar*>(message.data()+4));

    if (!data.isEmpty())
        message.append(data);

    qDebug() << "Sending reply to" << hex << command << "state" << state << "of length" << message.size();
    qDebug() << message.toHex();

    qint64 re = socket->write(message);
    Q_ASSERT(re == message.size());
    Q_UNUSED(re);
}
