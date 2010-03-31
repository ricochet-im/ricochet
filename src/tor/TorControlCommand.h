#ifndef TORCONTROLCOMMAND_H
#define TORCONTROLCOMMAND_H

#include <QObject>
#include <QByteArray>

namespace Tor
{

class TorControlCommand : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(TorControlCommand)

	friend class TorControlSocket;

public:
	const char * const keyword;

    TorControlCommand(const char *keyword);

	int statusCode() const { return pStatusCode; }

signals:
	void replyFinished();

protected:
	virtual void handleReply(int code, QByteArray &data, bool end) = 0;

private:
	int pStatusCode;

	void inputReply(int code, QByteArray &data, bool end);
};

}

#endif // TORCONTROLCOMMAND_H
