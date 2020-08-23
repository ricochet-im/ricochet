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

#ifndef TORCONTROLSOCKET_H
#define TORCONTROLSOCKET_H

#include <QTcpSocket>
#include <QQueue>

namespace Tor
{

class TorControlCommand;

class TorControlSocket : public QTcpSocket
{
Q_OBJECT
public:
    explicit TorControlSocket(QObject *parent = 0);
    virtual ~TorControlSocket();

    QString errorMessage() const { return m_errorMessage; }

    void registerEvent(const QByteArray &event, TorControlCommand *handler);

    void sendCommand(const QByteArray &data) { sendCommand(0, data); }
    void sendCommand(TorControlCommand *command, const QByteArray &data);

signals:
    void error(const QString &message);

private slots:
    void process();
    void clear();

private:
    QQueue<TorControlCommand*> commandQueue;
    QHash<QByteArray,TorControlCommand*> eventCommands;
    QString m_errorMessage;
    TorControlCommand *currentCommand;
    bool inDataReply;

    void setError(const QString &message);
};

}

#endif // TORCONTROLSOCKET_H
