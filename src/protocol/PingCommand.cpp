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

#include "PingCommand.h"

REGISTER_COMMAND_HANDLER(0x00, PingCommand)

PingCommand::PingCommand(QObject *parent)
    : ProtocolCommand(parent)
{
}

void PingCommand::send(ProtocolManager *to)
{
    prepareCommand(commandState(0));
    sendCommand(to, true);

    qDebug() << "Sent ping";
}

void PingCommand::process(CommandHandler &command)
{
    qDebug() << "Received ping with identifier" << command.identifier;
    command.sendReply(replyState(true, true, 0x00));
}

void PingCommand::processReply(quint8 state, const uchar *data, unsigned dataSize)
{
    Q_UNUSED(state);
    Q_UNUSED(data);
    Q_UNUSED(dataSize);

    qDebug() << "Received ping reply with state" << state;
}
