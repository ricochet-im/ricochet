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

#include "CommandHandler.h"
#include "ProtocolCommand.h"
#include <QtEndian>
#include <QtDebug>
#include <QTcpSocket>

CommandHandler::CommandFunc CommandHandler::handlerMap[256] = { 0 };

CommandHandler::CommandHandler(ContactUser *u, QTcpSocket *s, const uchar *m, unsigned mS)
    : user(u),
      data((mS > 6) ? QByteArray::fromRawData(reinterpret_cast<const char*>(m+6), mS-6) : QByteArray()),
      socket(s)
{
    Q_ASSERT(mS >= 6);
    command = m[2];
    state = m[3];
    identifier = qFromBigEndian<quint16>(m+4);

    CommandFunc handler = handlerMap[command];

    qDebug() << "Handling command 0x" << hex << command << "state" << state << "from socket"
            << (void*)s << "with handler" << (void*)handler;

    if (!handler)
    {
        sendReply(ProtocolCommand::UnknownCommand);
        return;
    }

    handler(*this);
}

void CommandHandler::sendReply(quint8 state, const QByteArray &data)
{
    QByteArray message;
    message.reserve(data.size() + 6);
    message.resize(6);

    /* One more than the length of the data.. */
    qToBigEndian(quint16(data.size() + 1), reinterpret_cast<uchar*>(message.data()));
    message[2] = command;
    message[3] = state;
    qToBigEndian(identifier, reinterpret_cast<uchar*>(message.data()+4));

    if (!data.isEmpty())
        message.append(data);

    qDebug() << "Sending reply to" << hex << command << "state" << state << "of length" << message.size();
    qDebug() << message.toHex();

    qint64 re = socket->write(message);
    Q_ASSERT(re == message.size());
}
