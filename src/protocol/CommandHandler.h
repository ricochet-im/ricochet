#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <QByteArray>
#include "core/ContactUser.h"

class QTcpSocket;

class CommandHandler
{
public:
	typedef void (*CommandFunc)(CommandHandler &cmd);

	template<typename T> static inline void registerHandler(quint8 command)
	{
		handlerMap[command] = &T::process;
	}

	/* Command information */
	ContactUser * const user;
	const QByteArray data;
	quint8 command, state;
	quint16 identifier;

    explicit CommandHandler(ContactUser *user, QTcpSocket *socket, const uchar *message, unsigned messageSize);

	bool isReplyWanted() const { return identifier != 0; }

	void sendReply(quint8 state, const QByteArray &data = QByteArray());

private:
	static CommandFunc handlerMap[256];

	QTcpSocket * const socket;
};

template<quint8 command, typename T> class RegisterCommandHandler
{
public:
	RegisterCommandHandler()
	{
		CommandHandler::registerHandler<T>(command);
	}
};

#define REGISTER_COMMAND_HANDLER(command,x) static RegisterCommandHandler<command,x> cmdHandlerReg;

#endif // COMMANDHANDLER_H
