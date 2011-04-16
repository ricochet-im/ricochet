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

#include "TorControlSocket.h"
#include "TorControlCommand.h"
#include <QDebug>

using namespace Tor;

TorControlSocket::TorControlSocket(QObject *parent) :
    QTcpSocket(parent)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(process()));
}

void TorControlSocket::sendCommand(TorControlCommand *command, const QByteArray &data)
{
    commandQueue.append(command);

    Q_ASSERT(data.endsWith("\r\n"));
    write(data);

    qDebug() << "torctrl: Sent" << data;
}

void TorControlSocket::process()
{
    for (;;)
    {
        if (!canReadLine())
            return;

        QByteArray line = readLine(5120);

        if (line.size() < 4 || !line.endsWith("\r\n"))
        {
            controlError(tr("Invalid control message syntax (may not be a Tor control port)"));
            return;
        }

        if (line[3] == '+')
        {
            controlError(tr("BUG: Data replies are not supported"));
            return;
        }

        int code = line.left(3).toInt();
        bool end = (line[3] == ' ');

        if (!end && line[3] != '-')
        {
            controlError(tr("Invalid or unrecognized syntax (may not be a Tor control port)"));
            return;
        }

        if (commandQueue.isEmpty())
        {
            qWarning("torctrl: Received unexpected data");
            return;
        }

        TorControlCommand *command = commandQueue.first();

        qDebug() << "torctrl: Received" << (end ? "final" : "intermediate") << "reply for"
                << (command ? command->keyword : "???") << "-" << code << line.mid(4, line.size() - 6);

        if (command)
        {
            QByteArray data = line.mid(4, line.size() - 6);
            command->inputReply(code, data, end);
        }

        if (end)
        {
            commandQueue.takeFirst();

            emit commandFinished(command);
            command->deleteLater();
        }
    }
}
