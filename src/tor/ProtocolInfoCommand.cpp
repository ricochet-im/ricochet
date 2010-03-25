#include "ProtocolInfoCommand.h"
#include "TorControlManager.h"
#include <QList>

using namespace Tor;

ProtocolInfoCommand::ProtocolInfoCommand(TorControlManager *m)
	: TorControlCommand("PROTOCOLINFO"), manager(m)
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
		QFlags<TorControlManager::AuthMethod> authMethods;

		QList<QByteArray> textMethods = data.mid(13, data.indexOf(' ', 13)).split(',');
		for (QList<QByteArray>::Iterator it = textMethods.begin(); it != textMethods.end(); ++it)
		{
			if (*it == "NULL")
				authMethods |= TorControlManager::AuthNull;
			else if (*it == "HASHEDPASSWORD")
				authMethods |= TorControlManager::AuthHashedPassword;
			else if (*it == "COOKIE")
				authMethods |= TorControlManager::AuthCookie;
		}

		manager->pAuthMethods = authMethods;
	}
	else if (data.startsWith("VERSION Tor="))
	{
		manager->pTorVersion = data.mid(12, data.indexOf(' ', 12));
	}
}
