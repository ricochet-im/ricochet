#include "ChatMessageCommand.h"
#include <QDateTime>
#include <QDataStream>

ChatMessageCommand::ChatMessageCommand(QObject *parent)
	: ProtocolCommand(parent)
{
}

void ChatMessageCommand::send(ProtocolManager *to, const QDateTime &timestamp, const QString &text)
{
	QByteArray encodedText = text.toUtf8();
	if (encodedText.size() > maxCommandData - 6)
	{
		Q_ASSERT_X(false, "ChatMessageCommand", "Message truncated when sending command");
		encodedText.resize(maxCommandData - 6);
	}

	prepareCommand(0x00, encodedText.size() + 6);

	QDataStream stream(&commandBuffer, QIODevice::WriteOnly);
	stream.setVersion(QDataStream::Qt_4_6);
	stream << (quint32)timestamp.secsTo(QDateTime::currentDateTime());
	stream << encodedText;

	sendCommand(to, true);
}

void ChatMessageCommand::process(quint8 state, quint16 identifier, const uchar *data, unsigned dataSize)
{
	if (state != 0x00)
	{
		/* XXX send reply */
		return;
	}

	QByteArray dataArray = QByteArray::fromRawData((const char*)data, dataSize);
	QDataStream stream(&dataArray, QIODevice::ReadOnly);
	stream.setVersion(QDataStream::Qt_4_6);

	quint32 timestamp;
	QByteArray encodedText;

	stream >> timestamp >> encodedText;
	if (stream.status() != QDataStream::Ok)
	{
		/* XXX send reply */
		qDebug() << "Received invalid chat message command";
		return;
	}

	qDebug() << "Received chat message (time delta" << timestamp << "):" << QString::fromUtf8(encodedText);
}

void ChatMessageCommand::processReply(quint8 state, const uchar *data, unsigned dataSize)
{
	Q_UNUSED(state);
	Q_UNUSED(data);
	Q_UNUSED(dataSize);
}
