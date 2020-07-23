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

#ifndef TORSOCKET_H
#define TORSOCKET_H

#include <QTcpSocket>
#include <QTimer>

namespace Tor {

/* Specialized QTcpSocket which makes connections over the SOCKS proxy
 * from a TorControl instance, automatically attempts reconnections, and
 * reacts to Tor's connectivity state.
 *
 * Use normal QTcpSocket/QAbstractSocket API. When a connection fails, it
 * will be retried automatically after the correct interval and when
 * connectivity is available.
 *
 * To fully disconnect, destroy the object, or call
 * setReconnectEnabled(false) and disconnect the socket with
 * disconnectFromHost or abort.
 *
 * The caller is responsible for resetting the attempt counter if a
 * connection was successful and reconnection will be used again.
 */
class TorSocket : public QTcpSocket
{
    Q_OBJECT

public:
    explicit TorSocket(QObject *parent = 0);
    virtual ~TorSocket();

    bool reconnectEnabled() const { return m_reconnectEnabled; }
    void setReconnectEnabled(bool enabled);
    int maxAttemptInterval() { return m_maxInterval; }
    void setMaxAttemptInterval(int interval);
    void resetAttempts();

    virtual void connectToHost(const QString &hostName, quint16 port, OpenMode openMode = ReadWrite, NetworkLayerProtocol protocol = AnyIPProtocol);
    virtual void connectToHost(const QHostAddress &address, quint16 port, OpenMode openMode = ReadWrite);

    QString hostName() const { return m_host; }
    quint16 port() const { return m_port; }

protected:
    virtual int reconnectInterval();

private slots:
    void reconnect();
    void connectivityChanged();
    void onFailed();

private:
    QString m_host;
    quint16 m_port;
    QTimer m_connectTimer;
    bool m_reconnectEnabled;
    int m_maxInterval;
    int m_connectAttempts;

    using QAbstractSocket::connectToHost;
};

}

#endif
