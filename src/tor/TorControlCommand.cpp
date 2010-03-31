#include "TorControlCommand.h"

using namespace Tor;

TorControlCommand::TorControlCommand(const char *kw)
	: keyword(kw), pStatusCode(0)
{
}

void TorControlCommand::inputReply(int code, QByteArray &data, bool end)
{
	pStatusCode = code;
	handleReply(code, data, end);

	if (end)
		emit replyFinished();
}
