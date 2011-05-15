/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
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

#include "ChatMessageCommand.h"
#include "CommandDataParser.h"
#include <QDateTime>
#include <QBuffer>
#include <QDebug>

REGISTER_COMMAND_HANDLER(0x10, ChatMessageCommand)

static const int maxMessageChars = 4000;

ChatMessageCommand::ChatMessageCommand(QObject *parent)
    : ProtocolCommand(parent), m_finalReplyState(0)
{
}

void ChatMessageCommand::send(ProtocolManager *to, const QDateTime &timestamp, const QString &text, quint16 lastReceived)
{
    prepareCommand(0x00, 1024);
    CommandDataParser builder(&commandBuffer);

    builder << (quint32)timestamp.secsTo(QDateTime::currentDateTime());
    builder << lastReceived;
    builder << text;

    sendCommand(to, true);

    m_messageText = text;
    m_messageTime = timestamp;

    ChatMessageData message = {
        timestamp, text, identifier(), lastReceived
    };

    to->user->outgoingChatMessage(message, this);
}

void ChatMessageCommand::process(CommandHandler &command)
{
    QString text;
    quint32 timestamp;
    quint16 priorMessageID;

    CommandDataParser parser(&command.data);
    parser >> timestamp >> priorMessageID >> text;
    if (!parser)
    {
        command.sendReply(CommandSyntaxError);
        return;
    }

    text.truncate(maxMessageChars);

    ChatMessageData message = {
        QDateTime::currentDateTime().addSecs(-qint64(timestamp)),
        text,
        command.identifier,
        priorMessageID
    };

    command.user->m_lastReceivedChatID = command.identifier;
    command.user->incomingChatMessage(message);

    command.sendReply(replyState(true, true, 0x00));
}

void ChatMessageCommand::processReply(quint8 state, const uchar *data, unsigned dataSize)
{
    Q_UNUSED(state);
    Q_UNUSED(data);
    Q_UNUSED(dataSize);

    qDebug() << "Received chat message reply" << hex << state;
    if (isFinal(state))
        m_finalReplyState = state;
}
