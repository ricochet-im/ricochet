/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CHATMESSAGECOMMAND_H
#define CHATMESSAGECOMMAND_H

#include "ProtocolCommand.h"
#include <QDateTime>

class ChatMessageCommand : public ProtocolCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(ChatMessageCommand)

public:
    explicit ChatMessageCommand(QObject *parent = 0);

    virtual quint8 command() const { return 0x10; }
    static void process(CommandHandler &command);

    quint8 finalReplyState() const { return m_finalReplyState; }

    void send(ProtocolManager *to, const QDateTime &timestamp, const QString &text, quint16 lastReceivedID = 0);
    QString messageText() const { return m_messageText; }
    QDateTime messageTime() const { return m_messageTime; }

protected:
    virtual void processReply(quint8 state, const uchar *data, unsigned dataSize);

private:
    QString m_messageText;
    QDateTime m_messageTime;
    quint8 m_finalReplyState;
};

struct ChatMessageData
{
    QDateTime when;
    QString text;
    quint16 messageID;
    quint64 priorMessageID;
};

#endif // CHATMESSAGECOMMAND_H
