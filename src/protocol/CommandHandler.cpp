#include "CommandHandler.h"
#include <QtEndian>
#include <QtDebug>

CommandHandler::CommandFunc CommandHandler::handlerMap[256] = { 0 };

CommandHandler::CommandHandler(ContactUser *u, QTcpSocket *s, const uchar *m, unsigned mS)
	: user(u),
	  data((mS > 6) ? QByteArray::fromRawData(reinterpret_cast<const char*>(m+6), mS-6) : QByteArray()),
	  socket(s)
{
	Q_ASSERT(mS >= 6);
	command = m[2];
	state = m[3];
	identifier = qFromBigEndian<quint16>(m+4);

	CommandFunc handler = handlerMap[command];

	qDebug() << "Handling command 0x" << hex << command << "(state" << state << ") from socket"
			<< (void*)s << " with handler" << (void*)handler;

	if (!handler)
	{
		sendReply(0x11);
		return;
	}

	handler(*this);
}

void CommandHandler::sendReply(quint8 state, const QByteArray &data)
{
	qDebug("Would send reply 0x%02x to command 0x%02x", state, command);
}
