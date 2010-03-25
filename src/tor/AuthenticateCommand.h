#ifndef AUTHENTICATECOMMAND_H
#define AUTHENTICATECOMMAND_H

#include "TorControlCommand.h"

namespace Tor
{

class AuthenticateCommand : public TorControlCommand
{
public:
	QByteArray statusMessage;

    AuthenticateCommand();

	QByteArray build(const QByteArray &data = QByteArray());

protected:
	virtual void handleReply(int code, QByteArray &data, bool end);
};

}

#endif // AUTHENTICATECOMMAND_H
