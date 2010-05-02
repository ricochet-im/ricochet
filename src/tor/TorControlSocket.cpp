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

#include "TorControlSocket.h"
#include "TorControlCommand.h"
#include <QDebug>

using namespace Tor;

TorControlSocket::TorControlSocket(QObject *parent) :
    QTcpSocket(parent)
{
	connect(this, SIGNAL(readyRead()), this, SLOT(process()));
}

void TorControlSocket::sendCommand(TorControlCommand *command, const QByteArray &data)
{
	commandQueue.append(command);

	Q_ASSERT(data.endsWith("\r\n"));
	write(data);

	qDebug() << "torctrl: Sent" << data;
}

void TorControlSocket::process()
{
	for (;;)
	{
		if (!canReadLine())
			return;

		QByteArray line = readLine(5120);

		if (line.size() < 4 || !line.endsWith("\r\n"))
		{
			controlError(tr("Invalid control message syntax (may not be a Tor control port)"));
			return;
		}

		if (line[3] == '+')
		{
			controlError(tr("BUG: Data replies are not supported"));
			return;
		}

		int code = line.left(3).toInt();
		bool end = (line[3] == ' ');

		if (!end && line[3] != '-')
		{
			controlError(tr("Invalid or unrecognized syntax (may not be a Tor control port)"));
			return;
		}

		if (commandQueue.isEmpty())
		{
			qWarning("torctrl: Received unexpected data");
			return;
		}

		TorControlCommand *command = commandQueue.first();

		qDebug() << "torctrl: Received" << (end ? "final" : "intermediate") << "reply for"
				<< (command ? command->keyword : "???") << "-" << code << line.mid(4, line.size() - 6);

		if (command)
		{
			QByteArray data = line.mid(4, line.size() - 6);
			command->inputReply(code, data, end);
		}

		if (end)
		{
			commandQueue.takeFirst();

			emit commandFinished(command);
			command->deleteLater();
		}
	}
}
