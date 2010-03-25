#include "AuthenticateCommand.h"

using namespace Tor;

AuthenticateCommand::AuthenticateCommand()
	: TorControlCommand("AUTHENTICATE")
{
}

QByteArray AuthenticateCommand::build(const QByteArray &data)
{
	if (data.isNull())
		return QByteArray("AUTHENTICATE\r\n");

	return QByteArray("AUTHENTICATE ") + data.toHex() + "\r\n";
}

void AuthenticateCommand::handleReply(int code, QByteArray &data, bool end)
{
	if (end)
		statusMessage = data;
}
