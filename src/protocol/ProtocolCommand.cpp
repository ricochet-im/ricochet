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
