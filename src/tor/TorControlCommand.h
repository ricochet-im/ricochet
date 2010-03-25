#ifndef TORCONTROLCOMMAND_H
#define TORCONTROLCOMMAND_H

#include <QByteArray>

namespace Tor
{

class TorControlCommand
{
	friend class TorControlSocket;

public:
	const char * const keyword;

    TorControlCommand(const char *keyword);

	int statusCode() const { return pStatusCode; }

protected:
	virtual void handleReply(int code, QByteArray &data, bool end) = 0;

	void setStatusCode(int c) { pStatusCode = c; }

private:
	int pStatusCode;
};

}

#endif // TORCONTROLCOMMAND_H
