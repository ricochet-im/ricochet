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

#ifndef TORCONTROL_H
#define TORCONTROL_H

#include "utils/PendingOperation.h"

class QNetworkProxy;

namespace Tor
{

class HiddenService;
class TorControlPrivate;

class TorControl : public QObject
{
    Q_OBJECT
public:
    // control status
    enum Status
    {
        Error = -1,
        NotConnected,
        Connecting,
        Authenticating,
        Connected
    };

    // daemon status
    enum TorStatus
    {
        TorUnknown,
        TorOffline,
        TorReady
    };

    explicit TorControl(QObject *parent = 0);

    /* Information */
    Status status() const;
    TorStatus torStatus() const;
    QString torVersion() const;
    bool torVersionAsNewAs(const QString &version) const;
    QString errorMessage() const;

    bool hasConnectivity() const;
    QHostAddress socksAddress() const;
    quint16 socksPort() const;
    QNetworkProxy connectionProxy();

    /* Authentication */
    void setAuthPassword(const QByteArray &password);

    /* Connection */
    bool isConnected() const { return status() == Connected; }
    void connect(const QHostAddress &address, quint16 port);

    /* Ownership means that tor is managed by this socket, and we
     * can shut it down, own its configuration, etc. */
    bool hasOwnership() const;
    void takeOwnership();

    /* Hidden Services */
    QList<HiddenService*> hiddenServices() const;
    void addHiddenService(HiddenService *service);

    QVariantMap bootstrapStatus() const;
    QObject *getConfiguration(const QString &options);
    QObject *setConfiguration(const QVariantMap &options);
    PendingOperation *saveConfiguration();

signals:
    void statusChanged(int newStatus, int oldStatus);
    void torStatusChanged(int newStatus, int oldStatus);
    void connected();
    void disconnected();
    void connectivityChanged();
    void bootstrapStatusChanged();
    void hasOwnershipChanged();

public slots:
    /* Instruct Tor to shutdown */
    void shutdown();
    /* Call shutdown(), and wait synchronously for the command to be written */
    void shutdownSync();

    void reconnect();

private:
    TorControlPrivate *d;
};

}
#endif // TORCONTROLMANAGER_H
