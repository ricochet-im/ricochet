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

#include "GetSecretCommand.h"
#include "CommandDataParser.h"
#include <QDebug>

REGISTER_COMMAND_HANDLER(0x01, GetSecretCommand)

GetSecretCommand::GetSecretCommand(QObject *parent)
    : ProtocolCommand(parent), user(0)
{
}

void GetSecretCommand::send(ProtocolManager *to)
{
    prepareCommand(0x00);
    sendCommand(to, true);

    user = to->user;
}

void GetSecretCommand::process(CommandHandler &command)
{
    QByteArray secret = command.user->readSetting("localSecret").toByteArray();
    if (secret.size() != 16)
    {
        command.sendReply(ProtocolCommand::InternalError);
        return;
    }

    command.sendReply(replyState(true, true, 0x00), secret);
}

void GetSecretCommand::processReply(quint8 state, const uchar *data, unsigned dataSize)
{
    Q_UNUSED(state);

    if (dataSize != 16 || !user)
        return;

    qDebug() << "Setting remote secret for user" << user->uniqueID << "from command response";

    user->writeSetting("remoteSecret", QByteArray((const char*)data, dataSize));
}
