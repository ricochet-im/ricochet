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

#include "utils/PendingOperation.h"
#include "TorControl.h"
#include "TorControlSocket.h"
#include "HiddenService.h"
#include "ProtocolInfoCommand.h"
#include "AuthenticateCommand.h"
#include "SetConfCommand.h"
#include "GetConfCommand.h"
#include "AddOnionCommand.h"
#include "utils/StringUtil.h"

#include "error.hpp"
#include "globals.hpp"
#include "signals.hpp"
using tego::g_globals;

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
    bool hasOwnership;

    TorControlPrivate(TorControl *parent);

    void setStatus(TorControl::Status status);
    void setTorStatus(TorControl::TorStatus status);

    void getTorInfo();
    void publishServices();

public slots:
    void socketConnected();
    void socketDisconnected();
    void socketError();

    void authenticateReply();
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
      status(TorControl::NotConnected), torStatus(TorControl::TorUnknown),
      hasOwnership(false)
{
    socket = new TorControlSocket(this);
    QObject::connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
    QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
    QObject::connect(socket, SIGNAL(error(QString)), this, SLOT(setError(QString)));
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

    g_globals.context->callback_registry_.emit_tor_control_status_changed(
        static_cast<tego_tor_control_status_t>(status));

    if (status == TorControl::Connected && old < TorControl::Connected)
        emit q->connected();
    else if (status < TorControl::Connected && old >= TorControl::Connected)
        emit q->disconnected();
}

void TorControlPrivate::setTorStatus(TorControl::TorStatus n)
{
    if (n == torStatus)
    {
        return;
    }

    TorControl::TorStatus old = torStatus;
    torStatus = n;
    emit q->torStatusChanged(torStatus, old);
    emit q->connectivityChanged();

    switch(torStatus)
    {
        case TorControl::TorUnknown:
            g_globals.context->callback_registry_.emit_tor_network_status_changed(tego_tor_network_status_unknown);
            break;
        case TorControl::TorOffline:
            g_globals.context->callback_registry_.emit_tor_network_status_changed(tego_tor_network_status_offline);
            break;
        case TorControl::TorReady:
            g_globals.context->callback_registry_.emit_tor_network_status_changed(tego_tor_network_status_ready);
            break;
    }


    if (torStatus == TorControl::TorReady)
{
        if (socksAddress.isNull())
        {
            // Request info again to read the SOCKS port
            getTorInfo();
        }
        else
        {
            g_globals.context->set_host_user_state(tego_host_user_state_online);
        }
    }
}

void TorControlPrivate::setError(const QString &message)
{
    errorMessage = message;
    setStatus(TorControl::Error);

    qWarning() << "torctrl: Error:" << errorMessage;

    auto tegoError = std::make_unique<tego_error>();
    tegoError->message = message.toStdString();
    g_globals.context->callback_registry_.emit_tor_error_occurred(
        tego_tor_error_origin_control,
        tegoError.release());

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

void TorControlPrivate::authenticateReply()
{
    AuthenticateCommand *command = qobject_cast<AuthenticateCommand*>(sender());
    Q_ASSERT(command);
    Q_ASSERT(status == TorControl::Authenticating);
    if (!command)
        return;

    if (!command->isSuccessful()) {
        setError(command->errorMessage());
        return;
    }

    qDebug() << "torctrl: Authentication successful";
    setStatus(TorControl::Connected);

    setTorStatus(TorControl::TorUnknown);

    TorControlCommand *clientEvents = new TorControlCommand;
    connect(clientEvents, &TorControlCommand::replyLine, this, &TorControlPrivate::statusEvent);
    socket->registerEvent("STATUS_CLIENT", clientEvents);

    getTorInfo();
    publishServices();

    // XXX Fix old configurations that would store unwanted options in torrc.
    // This can be removed some suitable amount of time after 1.0.4.
    if (hasOwnership)
        q->saveConfiguration();
}

void TorControlPrivate::socketConnected()
{
    Q_ASSERT(status == TorControl::Connecting);

    qDebug() << "torctrl: Connected socket; querying information";
    setStatus(TorControl::Authenticating);

    ProtocolInfoCommand *command = new ProtocolInfoCommand(q);
    connect(command, &TorControlCommand::finished, this, &TorControlPrivate::protocolInfoReply);
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
        connect(auth, &TorControlCommand::finished, this, &TorControlPrivate::authenticateReply);

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

    GetConfCommand *command = new GetConfCommand(GetConfCommand::GetInfo);
    connect(command, &TorControlCommand::finished, this, &TorControlPrivate::getTorInfoReply);

    QList<QByteArray> keys;
    keys << QByteArray("status/circuit-established") << QByteArray("status/bootstrap-phase");

    keys << QByteArray("net/listeners/socks");

    socket->sendCommand(command, command->build(keys));
}

void TorControlPrivate::getTorInfoReply()
{
    GetConfCommand *command = qobject_cast<GetConfCommand*>(sender());
    if (!command || !q->isConnected())
        return;

    QList<QByteArray> listenAddresses = splitQuotedStrings(command->get(QByteArray("net/listeners/socks")).toString().toLatin1(), ' ');
    for (QList<QByteArray>::Iterator it = listenAddresses.begin(); it != listenAddresses.end(); ++it) {
        QByteArray value = unquotedString(*it);
        int sepp = value.indexOf(':');
        QHostAddress address(QString::fromLatin1(value.mid(0, sepp)));
        quint16 port = static_cast<quint16>(value.mid(sepp+1).toUInt());

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
        g_globals.context->set_host_user_state(tego_host_user_state_online);
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

    // v3 works in all supported tor versions:
    // https://trac.torproject.org/projects/tor/wiki/org/teams/NetworkTeam/CoreTorReleases
    Q_ASSERT(q->torVersionAsNewAs(QStringLiteral("0.3.5")));

    foreach (HiddenService *service, services) {
        if (service->hostname().isEmpty())
            qDebug() << "torctrl: Creating a new hidden service";
        else
            qDebug() << "torctrl: Publishing hidden service" << service->hostname();
        AddOnionCommand *onionCommand = new AddOnionCommand(service);
        QObject::connect(onionCommand, &AddOnionCommand::succeeded, service, &HiddenService::servicePublished);
        socket->sendCommand(onionCommand, onionCommand->build());
    }
}

void TorControl::shutdown()
{
    if (!hasOwnership()) {
        qWarning() << "torctrl: Ignoring shutdown command for a tor instance I don't own";
        return;
    }

    d->socket->sendCommand("SIGNAL SHUTDOWN\r\n");
}

void TorControl::shutdownSync()
{
    if (!hasOwnership()) {
        qWarning() << "torctrl: Ignoring shutdown command for a tor instance I don't own";
        return;
    }

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

	// these functions just access 'bootstrapStatus' and parse out the relevant keys
	// a bit roundabout but better than duplicating the tag parsing logic
    auto progress = g_globals.context->get_tor_bootstrap_progress();
    auto tag = g_globals.context->get_tor_bootstrap_tag();

    g_globals.context->callback_registry_.emit_tor_bootstrap_status_changed(
        progress,
        tag);

    qDebug() << bootstrapStatus;
    emit q->bootstrapStatusChanged();
}

QObject *TorControl::getConfiguration(const QString &options)
{
    GetConfCommand *command = new GetConfCommand(GetConfCommand::GetConf);
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

namespace Tor {

class SaveConfigOperation : public PendingOperation
{
    Q_OBJECT

public:
    SaveConfigOperation(QObject *parent)
        : PendingOperation(parent), command(0)
    {
    }

    void start(TorControlSocket *socket)
    {
        Q_ASSERT(!command);
        command = new GetConfCommand(GetConfCommand::GetInfo);
        QObject::connect(command, &TorControlCommand::finished, this, &SaveConfigOperation::configTextReply);
        socket->sendCommand(command, command->build(QList<QByteArray>() << "config-text" << "config-file"));
    }

private slots:
    void configTextReply()
    {
        Q_ASSERT(command);
        if (!command)
            return;

        QString path = QFile::decodeName(command->get("config-file").toByteArray());
        if (path.isEmpty()) {
            finishWithError(QStringLiteral("Cannot write torrc without knowing its path"));
            return;
        }

        // Out of paranoia, refuse to write any file not named 'torrc', or if the
        // file doesn't exist
        QFileInfo fileInfo(path);
        if (fileInfo.fileName() != QStringLiteral("torrc") || !fileInfo.exists()) {
            finishWithError(QStringLiteral("Refusing to write torrc to unacceptable path %1").arg(path));
            return;
        }

        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            finishWithError(QStringLiteral("Failed opening torrc file for writing: %1").arg(file.errorString()));
            return;
        }

        // Remove these keys when writing torrc; they are set at runtime and contain
        // absolute paths or port numbers
        static const char *bannedKeys[] = {
            "ControlPortWriteToFile",
            "DataDirectory",
            "HiddenServiceDir",
            "HiddenServicePort",
            0
        };

        QVariantList configText = command->get("config-text").toList();
        foreach (const QVariant &value, configText) {
            QByteArray line = value.toByteArray();

            bool skip = false;
            for (const char **key = bannedKeys; *key; key++) {
                if (line.startsWith(*key)) {
                    skip = true;
                    break;
                }
            }
            if (skip)
                continue;

            file.write(line);
            file.write("\n");
        }

        if (!file.commit()) {
            finishWithError(QStringLiteral("Failed writing torrc: %1").arg(file.errorString()));
            return;
        }

        qDebug() << "torctrl: Wrote torrc file";
        finishWithSuccess();
    }

private:
    GetConfCommand *command;
};

}

PendingOperation *TorControl::saveConfiguration()
{
    if (!hasOwnership()) {
        qWarning() << "torctrl: Ignoring save configuration command for a tor instance I don't own";
        return 0;
    }

    SaveConfigOperation *operation = new SaveConfigOperation(this);
    QObject::connect(operation, &PendingOperation::finished, operation, &QObject::deleteLater);
    operation->start(d->socket);

    QQmlEngine::setObjectOwnership(operation, QQmlEngine::CppOwnership);
    return operation;
}

bool TorControl::hasOwnership() const
{
    return d->hasOwnership;
}

void TorControl::takeOwnership()
{
    d->hasOwnership = true;
    d->socket->sendCommand("TAKEOWNERSHIP\r\n");

    // Reset PID-based polling
    QVariantMap options;
    options[QStringLiteral("__OwningControllerProcess")] = QVariant();
    setConfiguration(options);
}

bool TorControl::torVersionAsNewAs(const QString &match) const
{
    QRegularExpression r(QStringLiteral("[.-]"));
    QStringList split = torVersion().split(r);
    QStringList matchSplit = match.split(r);

    for (int i = 0; i < matchSplit.size(); i++) {
        if (i >= split.size())
            return false;
        bool ok1 = false, ok2 = false;
        int currentVal = split[i].toInt(&ok1);
        int matchVal = matchSplit[i].toInt(&ok2);
        if (!ok1 || !ok2)
            return false;
        if (currentVal > matchVal)
            return true;
        if (currentVal < matchVal)
            return false;
    }

    // Versions are equal, up to the length of match
    return true;
}

#include "TorControl.moc"

