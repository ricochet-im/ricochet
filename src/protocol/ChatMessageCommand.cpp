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

#include "ChatMessageCommand.h"
#include "CommandDataParser.h"
#include "ui/ChatWidget.h"
#include <QDateTime>
#include <QBuffer>
#include <QDebug>

REGISTER_COMMAND_HANDLER(0x10, ChatMessageCommand)

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

    qDebug().nospace() << "Received chat message (time delta " << timestamp << ", prior message "
            << priorMessageID << "): " << text;

    ChatWidget *chat = ChatWidget::widgetForUser(command.user);
    chat->receiveMessage(QDateTime::currentDateTime().addSecs(-(int)timestamp), text, command.identifier, priorMessageID);

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
