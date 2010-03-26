#include "ProtocolCommand.h"
#include <QtDebug>

ProtocolCommand::ProtocolCommand(QObject *parent)
	: QObject(parent), identifier(0)
{
	qWarning("ProtocolCommand::identifier is not implemented");
}

bool ProtocolCommand::startUnbufferedReply(quint8 state)
{
	Q_UNUSED(state);
	qWarning() << "Received an invalid unbuffered reply for command 0x" << hex << command()
			<< " (no unbuffered reply expected)";
	return false;
}

int ProtocolCommand::processUnbufferedReply(const char *data, unsigned dataSize)
{
	Q_UNUSED(data);
	Q_UNUSED(dataSize);

	/* This is never called unless startUnbufferedReply is implemented, so any path reaching here is
	 * an implementation error. */
	Q_ASSERT_X(false, metaObject()->className(), "Command does not implement processUnbufferedReply");
	return -1;
}
