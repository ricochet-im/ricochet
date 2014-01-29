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
#include "TorControl.h"
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

Tor::TorControl *torManager = 0;

using namespace Tor;

namespace Tor {

class TorControlPrivate : public QObject
{
    Q_OBJECT

public:
    TorControl *q;

    TorControlSocket *socket;
    QHostAddress torAddress;
    QString errorMessage;
    QString torVersion;
    QByteArray authPassword;
    QHostAddress socksAddress;
    QList<HiddenService*> services;
    quint16 controlPort, socksPort;
    TorControl::Status status;
    TorControl::TorStatus torStatus;

    TorControlPrivate(TorControl *parent);

    void setStatus(TorControl::Status status);
    void setTorStatus(TorControl::TorStatus status);

    void getTorStatus();
    void getSocksInfo();
    void publishServices();

public slots:
    void socketConnected();
    void socketDisconnected();
    void socketError();

    void commandFinished(class TorControlCommand *command);

    void protocolInfoReply();
    void getTorStatusReply();
    void getSocksInfoReply();
    void setError(const QString &message);

    void statusEvent(int code, const QByteArray &data);
};

}

TorControl::TorControl(QObject *parent)
    : QObject(parent), d(new TorControlPrivate(this))
{
}

TorControlPrivate::TorControlPrivate(TorControl *parent)
    : QObject(parent), q(parent), controlPort(0), socksPort(0),
      status(TorControl::NotConnected), torStatus(TorControl::TorUnknown)
{
    socket = new TorControlSocket;
    QObject::connect(socket, SIGNAL(commandFinished(TorControlCommand*)), this,
                     SLOT(commandFinished(TorControlCommand*)));
    QObject::connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
    QObject::connect(socket, SIGNAL(controlError(QString)), this, SLOT(setError(QString)));
}

QNetworkProxy TorControl::connectionProxy()
{
    return QNetworkProxy(QNetworkProxy::Socks5Proxy, d->socksAddress.toString(), d->socksPort);
}

void TorControlPrivate::setStatus(TorControl::Status n)
{
    if (n == status)
        return;

    TorControl::Status old = status;
    status = n;

    if (old == TorControl::Error)
        errorMessage.clear();

    emit q->statusChanged(status, old);

    if (status == TorControl::Connected && old < TorControl::Connected)
        emit q->connected();
    else if (status < TorControl::Connected && old >= TorControl::Connected)
        emit q->disconnected();
}

void TorControlPrivate::setTorStatus(TorControl::TorStatus n)
{
    if (n == torStatus)
        return;

    TorControl::TorStatus old = torStatus;
    torStatus = n;

    emit q->torStatusChanged(torStatus, old);
}

void TorControlPrivate::setError(const QString &message)
{
    errorMessage = message;
    setStatus(TorControl::Error);

    qWarning() << "torctrl: Error:" << errorMessage;

    socket->abort();

    QTimer::singleShot(15000, q, SLOT(reconnect()));
}

TorControl::Status TorControl::status() const
{
    return d->status;
}

TorControl::TorStatus TorControl::torStatus() const
{
    return d->torStatus;
}

QString TorControl::torVersion() const
{
    return d->torVersion;
}

QString TorControl::errorMessage() const
{
    return d->errorMessage;
}

bool TorControl::isSocksReady() const
{
    return !d->socksAddress.isNull();
}

QHostAddress TorControl::socksAddress() const
{
    return d->socksAddress;
}

quint16 TorControl::socksPort() const
{
    return d->socksPort;
}

QList<HiddenService*> TorControl::hiddenServices() const
{
    return d->services;
}

void TorControl::setAuthPassword(const QByteArray &password)
{
    d->authPassword = password;
}

void TorControl::connect(const QHostAddress &address, quint16 port)
{
    if (status() > Connecting)
    {
        qDebug() << "Ignoring TorControl::connect due to existing connection";
        return;
    }

    d->torAddress = address;
    d->controlPort = port;
    d->setTorStatus(TorUnknown);

    bool b = d->socket->blockSignals(true);
    d->socket->abort();
    d->socket->blockSignals(b);

    d->setStatus(Connecting);
    d->socket->connectToHost(address, port);
}

void TorControl::reconnect()
{
    Q_ASSERT(!d->torAddress.isNull() && d->controlPort);
    if (d->torAddress.isNull() || !d->controlPort || status() >= Connecting)
        return;

    d->setStatus(Connecting);
    d->socket->connectToHost(d->torAddress, d->controlPort);
}

void TorControlPrivate::commandFinished(TorControlCommand *command)
{
    QLatin1String keyword(command->keyword);

    if (keyword == QLatin1String("AUTHENTICATE"))
    {
        Q_ASSERT(status == TorControl::Authenticating);

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
        setStatus(TorControl::Connected);

        setTorStatus(TorControl::TorUnknown);

        getTorStatus();
        getSocksInfo();
        publishServices();
    }
}

void TorControlPrivate::socketConnected()
{
    Q_ASSERT(status == TorControl::Connecting);

    qDebug() << "torctrl: Connected socket; querying information";
    setStatus(TorControl::Authenticating);

    ProtocolInfoCommand *command = new ProtocolInfoCommand(q);
    QObject::connect(command, SIGNAL(finished()), SLOT(protocolInfoReply()));
    socket->sendCommand(command, command->build());
}

void TorControlPrivate::socketDisconnected()
{
    /* Clear some internal state */
    torVersion.clear();
    socksAddress.clear();
    socksPort = 0;
    setTorStatus(TorControl::TorUnknown);

    /* This emits the disconnected() signal as well */
    setStatus(TorControl::NotConnected);
}

void TorControlPrivate::socketError()
{
    setError(tr("Connection failed: %1").arg(socket->errorString()));
}

void TorControlPrivate::protocolInfoReply()
{
    ProtocolInfoCommand *info = qobject_cast<ProtocolInfoCommand*>(sender());
    if (!info)
        return;

    torVersion = info->torVersion();

    if (status == TorControl::Authenticating)
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
                if (methods.testFlag(ProtocolInfoCommand::AuthHashedPassword) && !authPassword.isEmpty())
                {
                    qDebug() << "torctrl: Unable to read authentication cookie file:" << cookieError;
                    goto usePasswordAuth;
                }

                setError(tr("Unable to read authentication cookie file: %1").arg(cookieError));
                delete auth;
                return;
            }
        }
        else if (methods.testFlag(ProtocolInfoCommand::AuthHashedPassword) && !authPassword.isEmpty())
        {
            usePasswordAuth:
            qDebug() << "torctrl: Using hashed password authentication";
            data = auth->build(authPassword);
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

void TorControlPrivate::getTorStatus()
{
    Q_ASSERT(q->isConnected());

    TorControlCommand *clientEvents = new TorControlCommand("STATUS_CLIENT");
    connect(clientEvents, SIGNAL(replyLine(int,QByteArray,bool)), this, SLOT(statusEvent(int,QByteArray)));
    socket->registerEvent("STATUS_CLIENT", clientEvents);

    GetConfCommand *command = new GetConfCommand("GETINFO");
    QObject::connect(command, SIGNAL(finished()), this, SLOT(getTorStatusReply()));

    QList<QByteArray> keys;
    keys << QByteArray("status/circuit-established") << QByteArray("status/bootstrap-phase");

    socket->sendCommand(command, command->build(keys));
}

void TorControlPrivate::getTorStatusReply()
{
    GetConfCommand *command = qobject_cast<GetConfCommand*>(sender());
    if (!command || !q->isConnected())
        return;

    Q_ASSERT(QLatin1String(command->keyword) == QLatin1String("GETINFO"));

    QByteArray re;
    if (command->get(QByteArray("status/circuit-established"), re) && re.toInt() == 1)
    {
        qDebug() << "torctrl: Tor indicates that circuits have been established; state is TorReady";
        setTorStatus(TorControl::TorReady);
        return;
    }

    /* Not handled yet */
    if (command->get(QByteArray("status/bootstrap-phase"), re))
        qDebug() << "torctrl: bootstrap-phase:" << re;

    setTorStatus(TorControl::TorOffline);
}

void TorControlPrivate::getSocksInfo()
{
    Q_ASSERT(q->isConnected());

    /* If these are set in the config, they override the automatic behavior. */
    QHostAddress forceAddress(config->value("tor/socksIp").toString());
    quint16 port = (quint16)config->value("tor/socksPort").toUInt();

    if (!forceAddress.isNull() && port)
    {
        qDebug() << "torctrl: Using manually specified SOCKS connection settings";
        socksAddress = forceAddress;
        socksPort = port;
        emit q->socksReady();
        return;
    }

    qDebug() << "torctrl: Querying for SOCKS connection settings";

    GetConfCommand *command = new GetConfCommand;
    QObject::connect(command, SIGNAL(finished()), this, SLOT(getSocksInfoReply()));

    QList<QByteArray> options;
    options << QByteArray("SocksPort") << QByteArray("SocksListenAddress");

    socket->sendCommand(command, command->build(options));
}

void TorControlPrivate::getSocksInfoReply()
{
    GetConfCommand *command = qobject_cast<GetConfCommand*>(sender());
    if (!command || !q->isConnected())
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
        if (socksAddress.isNull() || address == socket->peerAddress())
        {
            socksAddress = address;
            socksPort = port;

            /* No need to parse the others if we got what we wanted */
            if (address == socket->peerAddress())
                break;
        }
    }

    if (socksAddress.isNull())
    {
        socksAddress.setAddress(QLatin1String("127.0.0.1"));
        socksPort = defaultPort;
    }

    qDebug().nospace() << "torctrl: SOCKS address is " << socksAddress.toString() << ":" << socksPort;
    emit q->socksReady();
}

void TorControl::addHiddenService(HiddenService *service)
{
    if (d->services.contains(service))
        return;

    d->services.append(service);
}

void TorControlPrivate::publishServices()
{
    Q_ASSERT(q->isConnected());
    if (services.isEmpty())
        return;

    if (config->value("core/neverPublishService", false).toBool())
    {
        qDebug() << "torctrl: Skipping service publication because neverPublishService is enabled";

        /* Call servicePublished under the assumption that they're published externally. */
        for (QList<HiddenService*>::Iterator it = services.begin(); it != services.end(); ++it)
            (*it)->servicePublished();

        return;
    }

    SetConfCommand *command = new SetConfCommand;
    QList<QPair<QByteArray,QByteArray> > settings;

    for (QList<HiddenService*>::Iterator it = services.begin(); it != services.end(); ++it)
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

void TorControl::shutdown()
{
    d->socket->sendCommand("SIGNAL SHUTDOWN\r\n");
}

void TorControl::shutdownSync()
{
    shutdown();
    while (d->socket->bytesToWrite())
    {
        if (!d->socket->waitForBytesWritten(5000))
            return;
    }
}

void TorControlPrivate::statusEvent(int code, const QByteArray &data)
{
    QList<QByteArray> tokens = splitQuotedStrings(data.trimmed(), ' ');
    if (tokens.size() < 3)
        return;

    qDebug() << "torctrl: status event:" << data.trimmed();

    if (tokens[2] == "CIRCUIT_ESTABLISHED") {
        setTorStatus(TorControl::TorReady);
    } else if (tokens[2] == "CIRCUIT_NOT_ESTABLISHED") {
        setTorStatus(TorControl::TorOffline);
    }
}

#include "TorControl.moc"

