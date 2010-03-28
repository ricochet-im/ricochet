#ifndef CHATMESSAGECOMMAND_H
#define CHATMESSAGECOMMAND_H

#include "ProtocolCommand.h"

class ChatMessageCommand : public ProtocolCommand
{
	Q_OBJECT
	Q_DISABLE_COPY(ChatMessageCommand)

public:
	explicit ChatMessageCommand(QObject *parent = 0);

	virtual quint8 command() const { return 0x10; }

	void send(ProtocolManager *to, const QDateTime &timestamp, const QString &text);

	static void process(quint8 state, quint16 identifier, const uchar *data, unsigned dataSize);

protected:
	virtual void processReply(quint8 state, const uchar *data, unsigned dataSize);
};

#endif // CHATMESSAGECOMMAND_H
