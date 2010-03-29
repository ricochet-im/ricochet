#include "PingCommand.h"

REGISTER_COMMAND_HANDLER(0x00, PingCommand)

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

void PingCommand::process(CommandHandler &command)
{
	qDebug() << "Received ping with identifier" << command.identifier;
	command.sendReply(0x00);
}

void PingCommand::processReply(quint8 state, const uchar *data, unsigned dataSize)
{
	Q_UNUSED(state);
	Q_UNUSED(data);
	Q_UNUSED(dataSize);

	qDebug() << "Received ping reply!";
}
