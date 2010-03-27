#ifndef PINGCOMMAND_H
#define PINGCOMMAND_H

#include "ProtocolCommand.h"

class PingCommand : public ProtocolCommand
{
	Q_OBJECT
	Q_DISABLE_COPY(PingCommand)

public:
    explicit PingCommand(QObject *parent = 0);

	virtual quint8 command() const { return 0x00; }

	void send(ProtocolManager *to);

	static void process(quint8 state, quint16 identifier, const uchar *data, unsigned dataSize);

protected:
	virtual void processReply(quint8 state, const uchar *data, unsigned dataSize);
};

#endif // PINGCOMMAND_H
