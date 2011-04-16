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

#include "main.h"
#include "TorControlManager.h"
#include "TorControlSocket.h"
#include "HiddenService.h"
#include "ProtocolInfoCommand.h"
#include "AuthenticateCommand.h"
#include "SetConfCommand.h"
#include "GetConfCommand.h"
#include "utils/StringUtil.h"
#include <QHostAddress>
#include <QDir>
#include <QNetworkProxy>
#include <QTimer>
#include <QDebug>

Tor::TorControlManager *torManager = 0;

using namespace Tor;

TorControlManager::TorControlManager(QObject *parent)
    : QObject(parent), pControlPort(0), pSocksPort(0), pStatus(NotConnected), pTorStatus(TorOffline)
{
    socket = new TorControlSocket;
    QObject::connect(socket, SIGNAL(commandFinished(TorControlCommand*)), this,
                     SLOT(commandFinished(TorControlCommand*)));
    QObject::connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
    QObject::connect(socket, SIGNAL(controlError(QString)), this, SLOT(setError(QString)));
}

QNetworkProxy TorControlManager::connectionProxy()
{
    return QNetworkProxy(QNetworkProxy::Socks5Proxy, pSocksAddress.toString(), pSocksPort);
}

void TorControlManager::setStatus(Status n)
{
    if (n == pStatus)
        return;

    Status old = pStatus;
    pStatus = n;

    if (old == Error)
        pErrorMessage.clear();

    emit statusChanged(pStatus, old);

    if (pStatus == Connected && old < Connected)
        emit connected();
    else if (pStatus < Connected && old >= Connected)
        emit disconnected();
}

void TorControlManager::setTorStatus(TorStatus n)
{
    if (n == pTorStatus)
        return;

    TorStatus old = pTorStatus;
    pTorStatus = n;

    emit torStatusChanged(pTorStatus, old);
}

void TorControlManager::setError(const QString &message)
{
    pErrorMessage = message;
    setStatus(Error);

    qWarning() << "torctrl: Error:" << pErrorMessage;

    socket->abort();

    QTimer::singleShot(15000, this, SLOT(reconnect()));
}

QString TorControlManager::statusText() const
{
    switch (pStatus)
    {
    case Error:
        return pErrorMessage.isEmpty() ? tr("An unknown error occurred") : pErrorMessage;
    case NotConnected:
        return tr("Not connected to Tor");
    case Connecting:
    case Authenticating:
        return tr("Connecting to Tor");
    case Connected:
        return tr("Connected to Tor");
    default:
        return QString();
    }
}

void TorControlManager::setAuthPassword(const QByteArray &password)
{
    pAuthPassword = password;
}

void TorControlManager::connect(const QHostAddress &address, quint16 port)
{
    if (status() > Connecting)
    {
        qDebug() << "Ignoring TorControlManager::connect due to existing connection";
        return;
    }

    pTorAddress = address;
    pControlPort = port;
    setTorStatus(TorUnknown);

    bool b = socket->blockSignals(true);
    socket->abort();
    socket->blockSignals(b);

    setStatus(Connecting);
    socket->connectToHost(address, port);
}

void TorControlManager::reconnect()
{
    Q_ASSERT(!pTorAddress.isNull() && pControlPort);
    if (pTorAddress.isNull() || !pControlPort || status() >= Connecting)
        return;

    setStatus(Connecting);
    socket->connectToHost(pTorAddress, pControlPort);
}

void TorControlManager::commandFinished(TorControlCommand *command)
{
    QLatin1String keyword(command->keyword);

    if (keyword == QLatin1String("AUTHENTICATE"))
    {
        Q_ASSERT(status() == Authenticating);

        if (command->statusCode() == 515)
        {
            setError(tr("Authentication failed - incorrect password"));
            return;
        }
        else if (command->statusCode() != 250)
        {
            setError(tr("Authentication failed (error %1)").arg(command->statusCode()));
            return;
        }

        qDebug() << "torctrl: Authentication successful";
        setStatus(Connected);

        setTorStatus(TorUnknown);

        getTorStatus();
        getSocksInfo();
        publishServices();
    }
}

void TorControlManager::socketConnected()
{
    Q_ASSERT(status() == Connecting);

    qDebug() << "torctrl: Connected socket; querying information";
    setStatus(Authenticating);

    ProtocolInfoCommand *command = new ProtocolInfoCommand(this);
    QObject::connect(command, SIGNAL(replyFinished()), SLOT(protocolInfoReply()));
    socket->sendCommand(command, command->build());
}

void TorControlManager::socketDisconnected()
{
    /* Clear some internal state */
    pTorVersion.clear();
    pSocksAddress.clear();
    pSocksPort = 0;
    setTorStatus(TorOffline);

    /* This emits the disconnected() signal as well */
    setStatus(NotConnected);
}

void TorControlManager::socketError()
{
    setError(tr("Connection failed: %1").arg(socket->errorString()));
}

void TorControlManager::protocolInfoReply()
{
    ProtocolInfoCommand *info = qobject_cast<ProtocolInfoCommand*>(sender());
    if (!info)
        return;

    pTorVersion = info->torVersion();

    if (status() == Authenticating)
    {
        AuthenticateCommand *auth = new AuthenticateCommand;
        QByteArray data;

        ProtocolInfoCommand::AuthMethods methods = info->authMethods();

        if (methods.testFlag(ProtocolInfoCommand::AuthNull))
        {
            qDebug() << "torctrl: Using null authentication";
            data = auth->build();
        }
        else if (methods.testFlag(ProtocolInfoCommand::AuthCookie) && !info->cookieFile().isEmpty())
        {
            QString cookieFile = info->cookieFile();
            QString cookieError;
            qDebug() << "torctrl: Using cookie authentication with file" << cookieFile;

            QFile file(cookieFile);
            if (file.open(QIODevice::ReadOnly))
            {
                QByteArray cookie = file.readAll();
                file.close();

                /* Simple test to avoid a vulnerability where any process listening on what we think is
                 * the control port could trick us into sending the contents of an arbitrary file */
                if (cookie.size() == 32)
                    data = auth->build(cookie);
                else
                    cookieError = tr("Unexpected file size");
            }
            else
                cookieError = file.errorString();

            if (!cookieError.isNull() || data.isNull())
            {
                /* If we know a password and password authentication is allowed, try using that instead.
                 * This is a strange corner case that will likely never happen in a normal configuration,
                 * but it has happened. */
                if (methods.testFlag(ProtocolInfoCommand::AuthHashedPassword) && !pAuthPassword.isEmpty())
                {
                    qDebug() << "torctrl: Unable to read authentication cookie file:" << cookieError;
                    goto usePasswordAuth;
                }

                setError(tr("Unable to read authentication cookie file: %1").arg(cookieError));
                delete auth;
                return;
            }
        }
        else if (methods.testFlag(ProtocolInfoCommand::AuthHashedPassword) && !pAuthPassword.isEmpty())
        {
            usePasswordAuth:
            qDebug() << "torctrl: Using hashed password authentication";
            data = auth->build(pAuthPassword);
        }
        else
        {
            if (methods.testFlag(ProtocolInfoCommand::AuthHashedPassword))
                setError(tr("Tor requires a control password to connect, but no password is configured."));
            else
                setError(tr("Tor is not configured to accept any supported authentication methods."));
            delete auth;
            return;
        }

        socket->sendCommand(auth, data);
    }
}

void TorControlManager::getTorStatus()
{
    Q_ASSERT(isConnected());

    GetConfCommand *command = new GetConfCommand("GETINFO");
    QObject::connect(command, SIGNAL(replyFinished()), this, SLOT(getTorStatusReply()));

    QList<QByteArray> keys;
    keys << QByteArray("status/circuit-established") << QByteArray("status/bootstrap-phase");

    socket->sendCommand(command, command->build(keys));
}

void TorControlManager::getTorStatusReply()
{
    GetConfCommand *command = qobject_cast<GetConfCommand*>(sender());
    if (!command || !isConnected())
        return;

    Q_ASSERT(QLatin1String(command->keyword) == QLatin1String("GETINFO"));

    QByteArray re;
    if (command->get(QByteArray("status/circuit-established"), re) && re.toInt() == 1)
    {
        qDebug() << "torctrl: Tor indicates that circuits have been established; state is TorReady";
        setTorStatus(TorReady);
        return;
    }

    /* Not handled yet */
    if (command->get(QByteArray("status/bootstrap-phase"), re))
        qDebug() << "torctrl: bootstrap-phase:" << re;

    setTorStatus(TorOffline);
}

void TorControlManager::getSocksInfo()
{
    Q_ASSERT(isConnected());

    /* If these are set in the config, they override the automatic behavior. */
    QHostAddress forceAddress(config->value("tor/socksIp").toString());
    quint16 port = (quint16)config->value("tor/socksPort").toUInt();

    if (!forceAddress.isNull() && port)
    {
        qDebug() << "torctrl: Using manually specified SOCKS connection settings";
        pSocksAddress = forceAddress;
        pSocksPort = port;
        emit socksReady();
        return;
    }

    qDebug() << "torctrl: Querying for SOCKS connection settings";

    GetConfCommand *command = new GetConfCommand;
    QObject::connect(command, SIGNAL(replyFinished()), this, SLOT(getSocksInfoReply()));

    QList<QByteArray> options;
    options << QByteArray("SocksPort") << QByteArray("SocksListenAddress");

    socket->sendCommand(command, command->build(options));
}

void TorControlManager::getSocksInfoReply()
{
    GetConfCommand *command = qobject_cast<GetConfCommand*>(sender());
    if (!command || !isConnected())
        return;

    /* If there is a SocksListenAddress line that is either null or has an IP with no port,
     * Tor is listening on that IP (default 127.0.0.1) and the value from SocksPort.
     *
     * If neither of those cases is true, SocksPort should be ignored, and Tor is listening
     * only on the IP:port pairs from SocksListenAddress.
     */

    QList<QByteArray> listenAddresses = command->getList(QByteArray("SocksListenAddress"));

    quint16 defaultPort = 0;
    QByteArray socksPortData;
    if (command->get(QByteArray("SocksPort"), socksPortData))
        defaultPort = (quint16)socksPortData.toUInt();
    if (!defaultPort)
        defaultPort = 9050;

    for (QList<QByteArray>::Iterator it = listenAddresses.begin(); it != listenAddresses.end(); ++it)
    {
        QHostAddress address;
        quint16 port = 0;

        if (!it->isNull())
        {
            int sepp = it->indexOf(':');
            address.setAddress(QString::fromLatin1(it->mid(0, sepp)));
            if (sepp >= 0)
                port = (quint16)it->mid(sepp+1).toUInt();
        }

        if (address.isNull())
            address.setAddress(QLatin1String("127.0.0.1"));
        if (!port)
            port = defaultPort;

        /* Use the first address that matches the one used for this control connection. If none do,
         * just use the first address and rely on the user to reconfigure if necessary (not a problem;
         * their setup is already very customized) */
        if (pSocksAddress.isNull() || address == socket->peerAddress())
        {
            pSocksAddress = address;
            pSocksPort = port;

            /* No need to parse the others if we got what we wanted */
            if (address == socket->peerAddress())
                break;
        }
    }

    if (pSocksAddress.isNull())
    {
        pSocksAddress.setAddress(QLatin1String("127.0.0.1"));
        pSocksPort = defaultPort;
    }

    qDebug().nospace() << "torctrl: SOCKS address is " << pSocksAddress.toString() << ":" << pSocksPort;
    emit socksReady();
}

void TorControlManager::addHiddenService(HiddenService *service)
{
    if (pServices.contains(service))
        return;

    pServices.append(service);
}

void TorControlManager::publishServices()
{
    Q_ASSERT(isConnected());
    if (pServices.isEmpty())
        return;

    if (config->value("core/neverPublishService", false).toBool())
    {
        qDebug() << "torctrl: Skipping service publication because neverPublishService is enabled";

        /* Call servicePublished under the assumption that they're published externally. */
        for (QList<HiddenService*>::Iterator it = pServices.begin(); it != pServices.end(); ++it)
            (*it)->servicePublished();

        return;
    }

    SetConfCommand *command = new SetConfCommand;
    QList<QPair<QByteArray,QByteArray> > settings;

    for (QList<HiddenService*>::Iterator it = pServices.begin(); it != pServices.end(); ++it)
    {
        HiddenService *service = *it;
        QDir dir(service->dataPath);

        qDebug() << "torctrl: Configuring hidden service at" << service->dataPath;

        settings.append(qMakePair(QByteArray("HiddenServiceDir"), dir.absolutePath().toLocal8Bit()));

        const QList<HiddenService::Target> &targets = service->targets();
        for (QList<HiddenService::Target>::ConstIterator tit = targets.begin(); tit != targets.end(); ++tit)
        {
            QString target = QString::fromLatin1("%1 %2:%3").arg(tit->servicePort)
                             .arg(tit->targetAddress.toString())
                             .arg(tit->targetPort);
            settings.append(qMakePair(QByteArray("HiddenServicePort"), target.toLatin1()));
        }

        QObject::connect(command, SIGNAL(setConfSucceeded()), service, SLOT(servicePublished()));
    }

    socket->sendCommand(command, command->build(settings));
}

void TorControlManager::shutdown()
{
    socket->sendCommand("SIGNAL SHUTDOWN\r\n");
}

void TorControlManager::shutdownSync()
{
    shutdown();
    while (socket->bytesToWrite())
    {
        if (!socket->waitForBytesWritten(5000))
            return;
    }
}
