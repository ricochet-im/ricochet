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

#include "GetSecretCommand.h"
#include "CommandDataParser.h"
#include "ProtocolConstants.h"
#include <QDebug>

REGISTER_COMMAND_HANDLER(0x01, GetSecretCommand)

GetSecretCommand::GetSecretCommand(QObject *parent)
    : ProtocolCommand(parent), user(0)
{
}

void GetSecretCommand::send(ProtocolSocket *to)
{
    prepareCommand(Protocol::commandState(0));
    sendCommand(to);

    user = to->user;
}

void GetSecretCommand::process(CommandHandler &command)
{
    QByteArray secret = command.user->settings()->read<Base64Encode>("localSecret");
    if (secret.size() != 16)
    {
        command.sendReply(Protocol::InternalError);
        return;
    }

    command.sendReply(Protocol::replyState(true, true, 0), secret);
}

void GetSecretCommand::processReply(quint8 state, const uchar *data, unsigned dataSize)
{
    Q_UNUSED(state);

    if (dataSize != 16 || !user)
        return;

    qDebug() << "Setting remote secret for user" << user->uniqueID << "from command response";

    QByteArray secret((const char*)data, dataSize);
    user->settings()->write("remoteSecret", Base64Encode(secret));
}
