#include "PingCommand.h"

PingCommand::PingCommand(QObject *parent)
	: ProtocolCommand(parent)
{
}

void PingCommand::send(ProtocolManager *to)
{
	prepareCommand(commandState(0));
	sendCommand(to, true);

	qDebug() << "Sent ping";
}

void PingCommand::process(quint8 state, quint16 identifier, const uchar *data, unsigned dataSize)
{
	Q_UNUSED(state);
	Q_UNUSED(data);
	Q_UNUSED(dataSize);

	qDebug() << "Received ping with identifier" << identifier;
}

void PingCommand::processReply(quint8 state, const uchar *data, unsigned dataSize)
{
	Q_UNUSED(state);
	Q_UNUSED(data);
	Q_UNUSED(dataSize);

	qDebug() << "Received ping reply!";
}
