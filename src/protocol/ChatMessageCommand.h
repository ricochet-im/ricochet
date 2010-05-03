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

    static void process(CommandHandler &command);

protected:
    virtual void processReply(quint8 state, const uchar *data, unsigned dataSize);
};

#endif // CHATMESSAGECOMMAND_H
