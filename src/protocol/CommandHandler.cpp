#include "CommandHandler.h"
#include "ProtocolCommand.h"
#include <QtEndian>
#include <QtDebug>
#include <QTcpSocket>

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

	qDebug() << "Handling command 0x" << hex << command << "state" << state << "from socket"
			<< (void*)s << "with handler" << (void*)handler;

	if (!handler)
	{
		sendReply(ProtocolCommand::UnknownCommand);
		return;
	}

	handler(*this);
}

void CommandHandler::sendReply(quint8 state, const QByteArray &data)
{
	QByteArray message;
	message.reserve(data.size() + 6);
	message.resize(6);

	/* One more than the length of the data.. */
	qToBigEndian(quint16(data.size() + 1), reinterpret_cast<uchar*>(message.data()));
	message[2] = command;
	message[3] = state;
	qToBigEndian(identifier, reinterpret_cast<uchar*>(message.data()+4));

	if (!data.isEmpty())
		message.append(data);

	qDebug() << "Sending reply to" << hex << command << "state" << state << "of length" << message.size();
	qDebug() << message.toHex();

	qint64 re = socket->write(message);
	Q_ASSERT(re == message.size());
}
