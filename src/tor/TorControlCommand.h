#ifndef TORCONTROLCOMMAND_H
#define TORCONTROLCOMMAND_H

#include <QByteArray>

namespace Tor
{

class TorControlCommand
{
public:
	const char * const keyword;

    TorControlCommand(const char *keyword);

	virtual void handleReply(int code, QByteArray &data, bool end) = 0;
};

}

#endif // TORCONTROLCOMMAND_H
