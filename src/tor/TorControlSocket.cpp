/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
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

TorControlSocket::TorControlSocket(QObject *parent)
    : QTcpSocket(parent), currentCommand(0), inDataReply(false)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(process()));
    connect(this, SIGNAL(disconnected()), this, SLOT(clear()));
}

TorControlSocket::~TorControlSocket()
{
    clear();
}

void TorControlSocket::sendCommand(TorControlCommand *command, const QByteArray &data)
{
    Q_ASSERT(data.endsWith("\r\n"));

    commandQueue.append(command);
    write(data);

    qDebug() << "torctrl: Sent" << data.trimmed();
}

void TorControlSocket::registerEvent(const QByteArray &event, TorControlCommand *command)
{
    eventCommands.insert(event, command);

    QByteArray data("SETEVENTS");
    foreach (const QByteArray &key, eventCommands.keys()) {
        data += ' ';
        data += key;
    }
    data += "\r\n";

    sendCommand(data);
}

void TorControlSocket::clear()
{
    qDeleteAll(commandQueue);
    commandQueue.clear();
    qDeleteAll(eventCommands);
    eventCommands.clear();
    inDataReply = false;
    currentCommand = 0;
}

void TorControlSocket::setError(const QString &message)
{
    m_errorMessage = message;
    emit error(message);
    abort();
}

void TorControlSocket::process()
{
    for (;;) {
        if (!canReadLine())
            return;

        QByteArray line = readLine(5120);
        if (!line.endsWith("\r\n")) {
            setError(QStringLiteral("Invalid control message syntax"));
            return;
        }
        line.chop(2);

        if (inDataReply) {
            if (line == ".") {
                inDataReply = false;
                if (currentCommand)
                    currentCommand->onDataFinished();
                currentCommand = 0;
            } else {
                if (currentCommand)
                    currentCommand->onDataLine(line);
            }
            continue;
        }

        if (line.size() < 4) {
            setError(QStringLiteral("Invalid control message syntax"));
            return;
        }

        int statusCode = line.left(3).toInt();
        char type = line[3];
        bool isFinalReply = (type == ' ');
        inDataReply = (type == '+');

        // Trim down to just data
        line = line.mid(4);

        if (!isFinalReply && !inDataReply && type != '-') {
            setError(QStringLiteral("Invalid control message syntax"));
            return;
        }

        // 6xx replies are asynchronous responses
        if (statusCode >= 600 && statusCode < 700) {
            if (!currentCommand) {
                int space = line.indexOf(' ');
                if (space > 0)
                    currentCommand = eventCommands.value(line.mid(0, space));

                if (!currentCommand) {
                    qWarning() << "torctrl: Ignoring unknown event";
                    continue;
                }
            }

            currentCommand->onReply(statusCode, line);
            if (isFinalReply) {
                currentCommand->onFinished(statusCode);
                currentCommand = 0;
            }
            continue;
        }

        if (commandQueue.isEmpty()) {
            qWarning() << "torctrl: Received unexpected data";
            continue;
        }

        TorControlCommand *command = commandQueue.first();
        if (command)
            command->onReply(statusCode, line);

        if (inDataReply) {
            currentCommand = command;
        } else if (isFinalReply) {
            commandQueue.takeFirst();
            if (command) {
                command->onFinished(statusCode);
                command->deleteLater();
            }
        }
    }
}
