#include "ProtocolInfoCommand.h"
#include <QList>

using namespace Tor;

ProtocolInfoCommand::ProtocolInfoCommand()
	: TorControlCommand("PROTOCOLINFO"), authMethod(AuthUnknown)
{
}

QByteArray ProtocolInfoCommand::build()
{
	return QByteArray("PROTOCOLINFO 1\r\n");
}

void ProtocolInfoCommand::handleReply(int code, QByteArray &data, bool end)
{
	if (code != 250)
		return;

	if (data.startsWith("AUTH METHODS="))
	{
		QList<QByteArray> methods = data.mid(13, data.indexOf(' ', 13)).split(',');
		for (QList<QByteArray>::Iterator it = methods.begin(); it != methods.end(); ++it)
		{
			if (*it == "NULL")
				authMethod |= AuthNull;
			else if (*it == "HASHEDPASSWORD")
				authMethod |= AuthHashedPassword;
			else if (*it == "COOKIE")
				authMethod |= AuthCookie;
		}
	}
	else if (data.startsWith("VERSION Tor="))
	{
		torVersion = data.mid(12, data.indexOf(' ', 12));
	}
}
