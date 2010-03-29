#include "CommandHandler.h"
#include <QtEndian>
#include <QtDebug>

CommandHandler::CommandFunc CommandHandler::handlerMap[256] = { 0 };

CommandHandler::CommandHandler(ContactUser *u, QTcpSocket *s, const uchar *m, unsigned mS)
	: user(u), data(QByteArray::fromRawData(reinterpret_cast<const char*>(m), mS)), socket(s)
{
	Q_ASSERT(data.size() >= 6);
	command = data[2];
	state = data[3];
	identifier = qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(data.constData())+4);

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
