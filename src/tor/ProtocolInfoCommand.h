#ifndef PROTOCOLINFOCOMMAND_H
#define PROTOCOLINFOCOMMAND_H

#include "TorControlCommand.h"
#include <QFlags>

namespace Tor
{

class TorControlManager;

class ProtocolInfoCommand : public TorControlCommand
{
public:
    ProtocolInfoCommand(TorControlManager *manager);

	QByteArray build();

protected:
	virtual void handleReply(int code, QByteArray &data, bool end);

private:
	TorControlManager *manager;
};

}

#endif // PROTOCOLINFOCOMMAND_H
