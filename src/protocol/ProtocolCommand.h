#ifndef PROTOCOLCOMMAND_H
#define PROTOCOLCOMMAND_H

#include <QObject>
#include <QByteArray>
#include "ProtocolManager.h"

class ProtocolCommand : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(ProtocolCommand)

	friend ProtocolManager;

public:
	static const int maxCommandData = 65540;

    explicit ProtocolCommand(QObject *parent = 0);

	virtual quint8 command() const = 0;
	quint16 identifier() const { return pIdentifier; }

protected:
	QByteArray commandBuffer;
	quint16 pIdentifier;

	virtual void processReply(quint8 state, const uchar *data, unsigned dataSize) = 0;

	/* Return value of false indicates that this cannot be handled and is a severe error */
	virtual bool beginUnbufferedReply(quint8 state);
	/* Consume data during an unbuffered reply; -1 indicates fatal error (use sparingly),
	 * 0 indicates that all data was consumed and more is expected, and any positive value indicates
	 * that the given number of bytes were consumed and the reply is now finished. */
	virtual int processUnbufferedReply(const uchar *data, unsigned dataSize);

	/* Returns the index of commandBuffer at which data begins; safe to call more than once */
	int prepareCommand(quint8 state, unsigned reserveSize = 0);
	void sendCommand(ProtocolManager *to, bool ordered);
};

inline quint8 commandState(quint8 value)
{
	Q_ASSERT(!(value & 0x80));
	return value & (~0x80);
}

inline quint8 replyState(bool success, quint8 value)
{
	Q_ASSERT(success || !(value & 0x20));
	Q_ASSERT(((value & 0x40) == 0x40) == success);
	if (success)
		return value | 0xc0;
	else
		return (value | 0x80) & (~0x40);
}

inline bool isReply(quint8 state)
{
	return (state & 0x80) == 0x80;
}

inline bool isSuccess(quint8 state)
{
	return (state & 0xc0) == 0xc0;
}

#endif // PROTOCOLCOMMAND_H
