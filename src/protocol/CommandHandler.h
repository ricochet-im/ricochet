/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

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
