#ifndef PROTOCOLINFOCOMMAND_H
#define PROTOCOLINFOCOMMAND_H

#include "TorControlCommand.h"
#include <QFlags>

namespace Tor
{

class ProtocolInfoCommand : public TorControlCommand
{
public:
	enum AuthMethod
	{
		AuthUnknown = 0,
		AuthNull = 0x1,
		AuthHashedPassword = 0x2,
		AuthCookie = 0x4
	};
	QFlags<AuthMethod> authMethod;
	QByteArray torVersion;

    ProtocolInfoCommand();

	QByteArray build();

protected:
	virtual void handleReply(int code, QByteArray &data, bool end);
};

}

#endif // PROTOCOLINFOCOMMAND_H
