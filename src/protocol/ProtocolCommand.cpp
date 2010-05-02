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

#include "ProtocolCommand.h"
#include <QtEndian>
#include <QtDebug>

ProtocolCommand::ProtocolCommand(QObject *parent)
	: QObject(parent)
{
}

int ProtocolCommand::prepareCommand(quint8 state, unsigned reserveSize)
{
	commandBuffer.reserve(reserveSize + 6);
	commandBuffer.resize(6);

	commandBuffer[2] = command();
	commandBuffer[3] = state;

	return 6;
}

void ProtocolCommand::sendCommand(ProtocolManager *to, bool ordered)
{
        Q_UNUSED(ordered);
	Q_ASSERT(commandBuffer.size() >= 6);
	Q_ASSERT(to);

	if (commandBuffer.size() > maxCommandData)
	{
		Q_ASSERT_X(false, metaObject()->className(), "Command data too large, would be truncated");
		qWarning() << "Truncated command" << metaObject()->className() << " (size " << commandBuffer.size()
				<< ")";
		commandBuffer.resize(maxCommandData);
	}

	ProtocolSocket *socket = to->primary();
	Q_ASSERT(socket);

	/* [2*length][1*command][1*state][2*identifier] */

	/* length is 1 more than the size of data non-inclusive of the header */
	qToBigEndian(quint16(commandBuffer.size() - 6 + 1), (uchar*)commandBuffer.data());

	pIdentifier = socket->getIdentifier();
	if (!pIdentifier)
		qFatal("Unable to acquire an identifier for command; report this");

	qToBigEndian(pIdentifier, (uchar*)commandBuffer.data() + 4);

	socket->sendCommand(this);
}

bool ProtocolCommand::beginUnbufferedReply(quint8 state)
{
	Q_UNUSED(state);
	qWarning() << "Received an invalid unbuffered reply for command 0x" << hex << command()
			<< " (no unbuffered reply expected)";
	return false;
}

int ProtocolCommand::processUnbufferedReply(const uchar *data, unsigned dataSize)
{
	Q_UNUSED(data);
	Q_UNUSED(dataSize);

	/* This is never called unless startUnbufferedReply is implemented, so any path reaching here is
	 * an implementation error. */
	Q_ASSERT_X(false, metaObject()->className(), "Command does not implement processUnbufferedReply");
	return -1;
}
