#ifndef PROTOCOLCOMMAND_H
#define PROTOCOLCOMMAND_H

#include <QObject>
#include <QByteArray>

class ProtocolCommand : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(ProtocolCommand)

public:
	const quint16 identifier;

    explicit ProtocolCommand(QObject *parent = 0);

	virtual quint8 command() const = 0;

protected:
	QByteArray commandBuffer;

	virtual void processReply(quint8 state, const char *data, unsigned dataSize) = 0;

	/* Return value of false indicates that this cannot be handled and is a severe error */
	virtual bool startUnbufferedReply(quint8 state);
	/* Consume data during an unbuffered reply; -1 indicates fatal error (use sparingly),
	 * 0 indicates that all data was consumed and more is expected, and any positive value indicates
	 * that the given number of bytes were consumed and the reply is now finished. */
	virtual int processUnbufferedReply(const char *data, unsigned dataSize);
};

#endif // PROTOCOLCOMMAND_H
