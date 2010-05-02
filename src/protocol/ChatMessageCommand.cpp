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

#include "ChatMessageCommand.h"
#include "CommandDataParser.h"
#include "ui/ChatWidget.h"
#include <QDateTime>
#include <QBuffer>

REGISTER_COMMAND_HANDLER(0x10, ChatMessageCommand)

ChatMessageCommand::ChatMessageCommand(QObject *parent)
	: ProtocolCommand(parent)
{
}

void ChatMessageCommand::send(ProtocolManager *to, const QDateTime &timestamp, const QString &text)
{
	prepareCommand(0x00, 1024);
	CommandDataParser builder(&commandBuffer);

	builder << (quint32)timestamp.secsTo(QDateTime::currentDateTime());
	builder << text;

	sendCommand(to, true);
}

void ChatMessageCommand::process(CommandHandler &command)
{
	quint32 timestamp;
	QString text;

	CommandDataParser parser(&command.data);
	parser >> timestamp >> text;
	if (!parser)
	{
		command.sendReply(CommandSyntaxError);
		return;
	}

	qDebug() << "Received chat message (time delta" << timestamp << "):" << text;

	ChatWidget *chat = ChatWidget::widgetForUser(command.user);
	chat->receiveMessage(QDateTime::currentDateTime().addSecs(-(int)timestamp), text);

	command.sendReply(replyState(true, true, 0x00));
}

void ChatMessageCommand::processReply(quint8 state, const uchar *data, unsigned dataSize)
{
	Q_UNUSED(state);
	Q_UNUSED(data);
	Q_UNUSED(dataSize);

	qDebug() << "Received chat message reply" << hex << state;
}
