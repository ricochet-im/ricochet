#include "PingCommand.h"

PingCommand::PingCommand(QObject *parent)
	: ProtocolCommand(parent)
{
}

void PingCommand::send(ProtocolManager *to)
{
	int p = prepareCommand(commandState(0));
	sendCommand(to, true);

	qDebug() << "Sent ping";
}

void PingCommand::processReply(quint8 state, const char *data, unsigned dataSize)
{
	Q_UNUSED(state);
	Q_UNUSED(data);
	Q_UNUSED(dataSize);

	qDebug() << "Received ping reply!";
}
