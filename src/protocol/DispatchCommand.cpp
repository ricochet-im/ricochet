#include "ProtocolManager.h"

#include "PingCommand.h"
#include "ChatMessageCommand.h"

void ProtocolManager::callCommand(quint8 command, quint8 state, quint16 identifier,
								  const uchar *data, unsigned dataSize)
{
	void (*handler)(quint8, quint16, const uchar*, unsigned) = 0;

	switch (command)
	{
	case 0x00:
		handler = &PingCommand::process;
		break;
	case 0x10:
		handler = &ChatMessageCommand::process;
		break;
	}

	if (!handler)
	{
		qDebug() << "Received unknown command 0x" << hex << command;
		qWarning() << "Unknown command reply is not yet implemented";
		return;
	}

	handler(state, identifier, data, dataSize);
}
