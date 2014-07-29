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

#include "TorControl.h"
#include "TorControlSocket.h"
#include "HiddenService.h"
#include "ProtocolInfoCommand.h"
#include "AuthenticateCommand.h"
#include "SetConfCommand.h"
#include "GetConfCommand.h"
#include "utils/StringUtil.h"
#include "utils/Settings.h"
#include <QHostAddress>
#include <QDir>
#include <QNetworkProxy>
#include <QQmlEngine>
#include <QTimer>
#include <QDebug>

Tor::TorControl *torControl = 0;

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
    QVariantMap bootstrapStatus;

    TorControlPrivate(TorControl *parent);

    void setStatus(TorControl::Status status);
    void setTorStatus(TorControl::TorStatus status);

    void getTorInfo();
    void publishServices();

public slots:
    void socketConnected();
    void socketDisconnected();
    void socketError();

    void commandFinished(class TorControlCommand *command);

    void protocolInfoReply();
    void getTorInfoReply();
    void setError(const QString &message);

    void statusEvent(int code, const QByteArray &data);
    void updateBootstrap(const QList<QByteArray> &data);
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
    emit q->connectivityChanged();

    if (torStatus == TorControl::TorReady && socksAddress.isNull()) {
        // Request info again to read the SOCKS port
        getTorInfo();
    }
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

bool TorControl::hasConnectivity() const
{
    return torStatus() == TorReady && !d->socksAddress.isNull();
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

QVariantMap TorControl::bootstrapStatus() const
{
    return d->bootstrapStatus;
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
            setError(QStringLiteral("Authentication failed - incorrect password"));
            return;
        }
        else if (command->statusCode() != 250)
        {
            setError(QStringLiteral("Authentication failed (error %1)").arg(command->statusCode()));
            return;
        }

        qDebug() << "torctrl: Authentication successful";
        setStatus(TorControl::Connected);

        setTorStatus(TorControl::TorUnknown);

        TorControlCommand *clientEvents = new TorControlCommand("STATUS_CLIENT");
        connect(clientEvents, SIGNAL(replyLine(int,QByteArray,bool)), this, SLOT(statusEvent(int,QByteArray)));
        socket->registerEvent("STATUS_CLIENT", clientEvents);

        getTorInfo();
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
    setError(QStringLiteral("Connection failed: %1").arg(socket->errorString()));
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
                    cookieError = QStringLiteral("Unexpected file size");
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

                setError(QStringLiteral("Unable to read authentication cookie file: %1").arg(cookieError));
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
                setError(QStringLiteral("Tor requires a control password to connect, but no password is configured."));
            else
                setError(QStringLiteral("Tor is not configured to accept any supported authentication methods."));
            delete auth;
            return;
        }

        socket->sendCommand(auth, data);
    }
}

void TorControlPrivate::getTorInfo()
{
    Q_ASSERT(q->isConnected());

    GetConfCommand *command = new GetConfCommand("GETINFO");
    QObject::connect(command, SIGNAL(finished()), this, SLOT(getTorInfoReply()));

    QList<QByteArray> keys;
    keys << QByteArray("status/circuit-established") << QByteArray("status/bootstrap-phase");

    /* If these are set in the config, they override the automatic behavior. */
    SettingsObject settings(QStringLiteral("tor"));
    QHostAddress forceAddress(settings.read("socksAddress").toString());
    quint16 port = (quint16)settings.read("socksPort").toInt();

    if (!forceAddress.isNull() && port) {
        qDebug() << "torctrl: Using manually specified SOCKS connection settings";
        socksAddress = forceAddress;
        socksPort = port;
        emit q->connectivityChanged();
    } else
        keys << QByteArray("net/listeners/socks");

    socket->sendCommand(command, command->build(keys));
}

void TorControlPrivate::getTorInfoReply()
{
    GetConfCommand *command = qobject_cast<GetConfCommand*>(sender());
    if (!command || !q->isConnected())
        return;

    Q_ASSERT(QLatin1String(command->keyword) == QLatin1String("GETINFO"));

    QList<QByteArray> listenAddresses = splitQuotedStrings(command->get(QByteArray("net/listeners/socks")).toString().toLatin1(), ' ');
    for (QList<QByteArray>::Iterator it = listenAddresses.begin(); it != listenAddresses.end(); ++it) {
        QByteArray value = unquotedString(*it);
        int sepp = value.indexOf(':');
        QHostAddress address(QString::fromLatin1(value.mid(0, sepp)));
        quint16 port = (quint16)value.mid(sepp+1).toUInt();

        /* Use the first address that matches the one used for this control connection. If none do,
         * just use the first address and rely on the user to reconfigure if necessary (not a problem;
         * their setup is already very customized) */
        if (socksAddress.isNull() || address == socket->peerAddress()) {
            socksAddress = address;
            socksPort = port;
            if (address == socket->peerAddress())
                break;
        }
    }

    /* It is not immediately an error to have no SOCKS address; when DisableNetwork is set there won't be a
     * listener yet. To handle that situation, we'll try to read the socks address again when TorReady state
     * is reached. */
    if (!socksAddress.isNull()) {
        qDebug().nospace() << "torctrl: SOCKS address is " << socksAddress.toString() << ":" << socksPort;
        emit q->connectivityChanged();
    }

    if (command->get(QByteArray("status/circuit-established")).toInt() == 1) {
        qDebug() << "torctrl: Tor indicates that circuits have been established; state is TorReady";
        setTorStatus(TorControl::TorReady);
    } else {
        setTorStatus(TorControl::TorOffline);
    }

    QByteArray bootstrap = command->get(QByteArray("status/bootstrap-phase")).toString().toLatin1();
    if (!bootstrap.isEmpty())
        updateBootstrap(splitQuotedStrings(bootstrap, ' '));
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

    SettingsObject settings(QStringLiteral("tor"));
    if (settings.read("neverPublishServices").toBool())
    {
        qDebug() << "torctrl: Skipping service publication because neverPublishService is enabled";

        /* Call servicePublished under the assumption that they're published externally. */
        for (QList<HiddenService*>::Iterator it = services.begin(); it != services.end(); ++it)
            (*it)->servicePublished();

        return;
    }

    SetConfCommand *command = new SetConfCommand;
    QList<QPair<QByteArray,QByteArray> > torConfig;

    for (QList<HiddenService*>::Iterator it = services.begin(); it != services.end(); ++it)
    {
        HiddenService *service = *it;
        QDir dir(service->dataPath);

        qDebug() << "torctrl: Configuring hidden service at" << service->dataPath;

        torConfig.append(qMakePair(QByteArray("HiddenServiceDir"), dir.absolutePath().toLocal8Bit()));

        const QList<HiddenService::Target> &targets = service->targets();
        for (QList<HiddenService::Target>::ConstIterator tit = targets.begin(); tit != targets.end(); ++tit)
        {
            QString target = QString::fromLatin1("%1 %2:%3").arg(tit->servicePort)
                             .arg(tit->targetAddress.toString())
                             .arg(tit->targetPort);
            torConfig.append(qMakePair(QByteArray("HiddenServicePort"), target.toLatin1()));
        }

        QObject::connect(command, SIGNAL(setConfSucceeded()), service, SLOT(servicePublished()));
    }

    socket->sendCommand(command, command->build(torConfig));
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
    Q_UNUSED(code);

    QList<QByteArray> tokens = splitQuotedStrings(data.trimmed(), ' ');
    if (tokens.size() < 3)
        return;

    qDebug() << "torctrl: status event:" << data.trimmed();

    if (tokens[2] == "CIRCUIT_ESTABLISHED") {
        setTorStatus(TorControl::TorReady);
    } else if (tokens[2] == "CIRCUIT_NOT_ESTABLISHED") {
        setTorStatus(TorControl::TorOffline);
    } else if (tokens[2] == "BOOTSTRAP") {
        tokens.takeFirst();
        updateBootstrap(tokens);
    }
}

void TorControlPrivate::updateBootstrap(const QList<QByteArray> &data)
{
    bootstrapStatus.clear();
    // WARN or NOTICE
    bootstrapStatus[QStringLiteral("severity")] = data.value(0);
    for (int i = 1; i < data.size(); i++) {
        int equals = data[i].indexOf('=');
        QString key = QString::fromLatin1(data[i].mid(0, equals));
        QString value;
        if (equals >= 0)
            value = QString::fromLatin1(unquotedString(data[i].mid(equals + 1)));
        bootstrapStatus[key.toLower()] = value;
    }

    qDebug() << bootstrapStatus;
    emit q->bootstrapStatusChanged();
}

QObject *TorControl::getConfiguration(const QString &options)
{
    GetConfCommand *command = new GetConfCommand;
    d->socket->sendCommand(command, command->build(options.toLatin1()));

    QQmlEngine::setObjectOwnership(command, QQmlEngine::CppOwnership);
    return command;
}

QObject *TorControl::setConfiguration(const QVariantMap &options)
{
    SetConfCommand *command = new SetConfCommand;
    command->setResetMode(true);
    d->socket->sendCommand(command, command->build(options));

    QQmlEngine::setObjectOwnership(command, QQmlEngine::CppOwnership);
    return command;
}

QObject *TorControl::saveConfiguration()
{
    TorControlCommand *command = new TorControlCommand("SAVECONF");
    d->socket->sendCommand(command, "SAVECONF\r\n");

    QQmlEngine::setObjectOwnership(command, QQmlEngine::CppOwnership);
    return command;
}

void TorControl::takeOwnership()
{
    TorControlCommand *command = new TorControlCommand("TAKEOWNERSHIP");
    d->socket->sendCommand(command, "TAKEOWNERSHIP\r\n");

    // Reset PID-based polling
    QVariantMap options;
    options[QStringLiteral("__OwningControllerProcess")] = QVariant();
    setConfiguration(options);
}

#include "TorControl.moc"

