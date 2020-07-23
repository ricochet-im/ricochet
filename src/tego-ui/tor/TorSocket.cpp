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

#include "TorSocket.h"
#include "TorControl.h"
#include <QNetworkProxy>

using namespace Tor;

TorSocket::TorSocket(QObject *parent)
    : QTcpSocket(parent)
    , m_port(0)
    , m_reconnectEnabled(true)
    , m_maxInterval(900)
    , m_connectAttempts(0)
{
    connect(torControl, SIGNAL(connectivityChanged()), SLOT(connectivityChanged()));
    connect(&m_connectTimer, SIGNAL(timeout()), SLOT(reconnect()));
    connect(this, SIGNAL(disconnected()), SLOT(onFailed()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(onFailed()));

    m_connectTimer.setSingleShot(true);
    connectivityChanged();
}

TorSocket::~TorSocket()
{
}

void TorSocket::setReconnectEnabled(bool enabled)
{
    if (enabled == m_reconnectEnabled)
        return;

    m_reconnectEnabled = enabled;
    if (m_reconnectEnabled) {
        m_connectAttempts = 0;
        reconnect();
    } else {
        m_connectTimer.stop();
    }
}

void TorSocket::setMaxAttemptInterval(int interval)
{
    m_maxInterval = interval;
}

void TorSocket::resetAttempts()
{
    m_connectAttempts = 0;
    if (m_connectTimer.isActive()) {
        m_connectTimer.stop();
        m_connectTimer.start(reconnectInterval() * 1000);
    }
}

int TorSocket::reconnectInterval()
{
    int delay = 0;
    if (m_connectAttempts <= 4)
        delay = 30;
    else if (m_connectAttempts <= 6)
        delay = 120;
    else
        delay = m_maxInterval;

    return qMin(delay, m_maxInterval);
}

void TorSocket::reconnect()
{
    if (!torControl->hasConnectivity() || !reconnectEnabled())
        return;

    m_connectTimer.stop();
    if (!m_host.isEmpty() && m_port) {
        qDebug() << "Attempting reconnection of socket to" << m_host << m_port;
        connectToHost(m_host, m_port);
    }
}

void TorSocket::connectivityChanged()
{
    if (torControl->hasConnectivity()) {
        setProxy(torControl->connectionProxy());
        if (state() == QAbstractSocket::UnconnectedState)
            reconnect();
    } else {
        m_connectTimer.stop();
        m_connectAttempts = 0;
    }
}

void TorSocket::connectToHost(const QString &hostName, quint16 port, OpenMode openMode,
        NetworkLayerProtocol protocol)
{
    m_host = hostName;
    m_port = port;

    if (!torControl->hasConnectivity())
        return;

    if (proxy() != torControl->connectionProxy())
        setProxy(torControl->connectionProxy());

    QAbstractSocket::connectToHost(hostName, port, openMode, protocol);
}

void TorSocket::connectToHost(const QHostAddress &address, quint16 port, OpenMode openMode)
{
    TorSocket::connectToHost(address.toString(), port, openMode);
}

void TorSocket::onFailed()
{
    // Make sure the internal connection to the SOCKS proxy is closed
    // Otherwise reconnect attempts will fail (#295)
    close();

    if (reconnectEnabled() && !m_connectTimer.isActive()) {
        m_connectAttempts++;
        m_connectTimer.start(reconnectInterval() * 1000);
        qDebug() << "Reconnecting socket to" << m_host << m_port << "in" << m_connectTimer.interval() / 1000 << "seconds";
    }
}
