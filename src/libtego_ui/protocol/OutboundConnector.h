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

#ifndef PROTOCOL_OUTBOUNDCONNECTOR_H
#define PROTOCOL_OUTBOUNDCONNECTOR_H

#include "Connection.h"
#include "utils/CryptoKey.h"

namespace Protocol
{

class OutboundConnectorPrivate;

/* Manages making and authenticating an outbound connection to peers
 *
 * OutboundConnector handles the process of establishing a connection
 * to a remote hidden service host (with appropriate timeout and retry
 * behavior) and authenticating itself. Once the connection is
 * established and authenticated, the ready() signal is emitted.
 */
class OutboundConnector : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutboundConnector)
    Q_ENUMS(Status)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(bool isActive READ isActive NOTIFY isActiveChanged)

public:
    enum Status {
        Inactive,
        Connecting,
        Initializing,
        Authenticating,
        Ready,
        Error
    };

    explicit OutboundConnector(QObject *parent);
    virtual ~OutboundConnector();

    Status status() const;
    bool isActive() const;
    QString errorMessage() const;

    bool connectToHost(const QString &hostname, quint16 port);
    void setAuthPrivateKey(const CryptoKey &key);

    /* Take ownership of the Connection object when Ready
     *
     * This function is only valid in the Ready state.
     * OutboundConnector will release ownership of the connection
     * and reset to the inactive state.
     */
    QSharedPointer<Connection> takeConnection();

public slots:
    void abort();

signals:
    void ready();
    void statusChanged();
    void isActiveChanged();

    /* Hack to allow sending an upgrade message to peers with old
     * software versions that don't have a good way to handle this
     * sort of situation.
     */
    void oldVersionNegotiated(QTcpSocket *socket);

private:
    OutboundConnectorPrivate *d;
};

}

#endif
