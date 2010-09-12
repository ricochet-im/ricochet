/* Torsion - http://github.com/special/torsion
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#ifndef TORCONTROLMANAGER_H
#define TORCONTROLMANAGER_H

#include <QObject>
#include <QHostAddress>

class QNetworkProxy;

namespace Tor
{

class HiddenService;

class TorControlManager : public QObject
{
    Q_OBJECT
    friend class ProtocolInfoCommand;

public:
    enum Status
    {
        Error = -1,
        NotConnected,
        Connecting,
        Authenticating,
        Connected
    };

    enum TorStatus
    {
        TorUnknown,
        TorOffline,
        TorBootstrapping,
        TorReady,
    };

    explicit TorControlManager(QObject *parent = 0);

    /* Information */
    Status status() const { return pStatus; }
    TorStatus torStatus() const { return (pStatus == Connected) ? pTorStatus : TorOffline; }
    QString torVersion() const { return pTorVersion; }
    QString statusText() const;

    bool isSocksReady() const { return !pSocksAddress.isNull(); }
    QHostAddress socksAddress() const { return pSocksAddress; }
    quint16 socksPort() const { return pSocksPort; }
    QNetworkProxy connectionProxy();

    /* Authentication */
    void setAuthPassword(const QByteArray &password);

    /* Connection */
    bool isConnected() const { return status() == Connected; }
    void connect(const QHostAddress &address, quint16 port);

    /* Hidden Services */
    const QList<HiddenService*> &hiddenServices() const { return pServices; }
    void addHiddenService(HiddenService *service);

signals:
    void statusChanged(int newStatus, int oldStatus);
    void torStatusChanged(int newStatus, int oldStatus);
    void connected();
    void disconnected();
    void socksReady();

public slots:
    /* Instruct Tor to shutdown */
    void shutdown();
    /* Call shutdown(), and wait synchronously for the command to be written */
    void shutdownSync();

    void reconnect();

private slots:
    void socketConnected();
    void socketDisconnected();
    void socketError();

    void commandFinished(class TorControlCommand *command);

    void protocolInfoReply();
    void getTorStatusReply();
    void getSocksInfoReply();

    void setError(const QString &message);

private:
    class TorControlSocket *socket;
    QHostAddress pTorAddress;
    QString pErrorMessage;
    QString pTorVersion;
    QByteArray pAuthPassword;
    QHostAddress pSocksAddress;
    QList<HiddenService*> pServices;
    quint16 pControlPort, pSocksPort;
    Status pStatus;
    TorStatus pTorStatus;

    void setStatus(Status status);
    void setTorStatus(TorStatus status);

    void getTorStatus();
    void getSocksInfo();
    void publishServices();
};

}

extern Tor::TorControlManager *torManager;

#endif // TORCONTROLMANAGER_H
