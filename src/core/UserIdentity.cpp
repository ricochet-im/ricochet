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

#include "UserIdentity.h"
#include "tor/TorControl.h"
#include "tor/HiddenService.h"
#include "core/ContactIDValidator.h"
#include "protocol/Connection.h"
#include "utils/Useful.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QBuffer>
#include <QDir>

using namespace Protocol;

UserIdentity::UserIdentity(int id, QObject *parent)
    : QObject(parent)
    , uniqueID(id)
    , contacts(this)
    , m_settings(0)
    , m_hiddenService(0)
    , m_incomingServer(0)
{
    m_settings = new SettingsObject(QStringLiteral("identity"), this);
    connect(m_settings, &SettingsObject::modified, this, &UserIdentity::onSettingsModified);

    setupService();

    contacts.loadFromSettings();
}

UserIdentity *UserIdentity::createIdentity(int uniqueID, const QString &dataDirectory)
{
    // There is actually no support for multiple identities currently.
    Q_ASSERT(uniqueID == 0);
    if (uniqueID != 0)
        return 0;

    SettingsObject settings(QStringLiteral("identity"));
    settings.write("initializing", true);
    if (dataDirectory.isEmpty())
        settings.write("dataDirectory", QString::fromLatin1("data-%1").arg(uniqueID));
    else
        settings.write("dataDirectory", dataDirectory);

    return new UserIdentity(uniqueID);
}

// TODO: Handle the error cases of this function in a useful way
void UserIdentity::setupService()
{
    QString keyData = m_settings->read("serviceKey").toString();
    QString legacyDir = m_settings->read("dataDirectory").toString();

    if (!keyData.isEmpty()) {
        CryptoKey key;
        if (!key.loadFromData(QByteArray::fromBase64(keyData.toLatin1()), CryptoKey::PrivateKey, CryptoKey::DER)) {
            qWarning() << "Cannot load service key from configuration";
            return;
        }

        m_hiddenService = new Tor::HiddenService(key, legacyDir, this);
    } else if (!legacyDir.isEmpty() && QFile::exists(legacyDir + QLatin1String("/private_key"))) {
        qDebug() << "Attempting to load key from legacy filesystem format in" << legacyDir;

        CryptoKey key;
        if (!key.loadFromFile(legacyDir + QLatin1String("/private_key"), CryptoKey::PrivateKey)) {
            qWarning() << "Cannot load legacy format key from" << legacyDir << "for conversion";
            return;
        } else {
            keyData = QString::fromLatin1(key.encodedPrivateKey(CryptoKey::DER).toBase64());
            m_settings->write("serviceKey", keyData);
            m_hiddenService = new Tor::HiddenService(key, legacyDir, this);
        }
    } else if (!m_settings->read("initializing").toBool()) {
        qWarning() << "Missing private key for initialized identity";
        return;
    } else {
        m_hiddenService = new Tor::HiddenService(legacyDir, this);
        connect(m_hiddenService, &Tor::HiddenService::privateKeyChanged, this,
            [&]() {
                QString key = QString::fromLatin1(m_hiddenService->privateKey().encodedPrivateKey(CryptoKey::DER).toBase64());
                m_settings->write("serviceKey", key);
            }
        );
    }

    Q_ASSERT(m_hiddenService);
    connect(m_hiddenService, SIGNAL(statusChanged(int,int)), SLOT(onStatusChanged(int,int)));

    // Generally, these are not used, and we bind to localhost and port 0
    // for an automatic (and portable) selection.
    QHostAddress address(m_settings->read("localListenAddress").toString());
    if (address.isNull())
        address = QHostAddress::LocalHost;
    quint16 port = (quint16)m_settings->read("localListenPort").toInt();

    m_incomingServer = new QTcpServer(this);
    if (!m_incomingServer->listen(address, port)) {
        // XXX error case
        qWarning() << "Failed to open incoming socket:" << m_incomingServer->errorString();
        return;
    }

    connect(m_incomingServer, &QTcpServer::newConnection, this, &UserIdentity::onIncomingConnection);

    m_hiddenService->addTarget(9878, m_incomingServer->serverAddress(), m_incomingServer->serverPort());
    torControl->addHiddenService(m_hiddenService);
}

SettingsObject *UserIdentity::settings()
{
    return m_settings;
}

QString UserIdentity::hostname() const
{
    return m_hiddenService ? m_hiddenService->hostname() : QString();
}

QString UserIdentity::contactID() const
{
    return ContactIDValidator::idFromHostname(hostname());
}

QString UserIdentity::nickname() const
{
    return m_settings->read("nickname").toString();
}

void UserIdentity::setNickname(const QString &nick)
{
    m_settings->write("nickname", nick);
}

void UserIdentity::onSettingsModified(const QString &key, const QJsonValue &value)
{
    Q_UNUSED(value);
    if (key == QLatin1String("nickname"))
        emit nicknameChanged();
}

void UserIdentity::onStatusChanged(int newStatus, int oldStatus)
{
    if (oldStatus == Tor::HiddenService::NotCreated && newStatus > oldStatus)
    {
        m_settings->write("initializing", QJsonValue::Undefined);
        emit contactIDChanged();
    }
    emit statusChanged();
}

bool UserIdentity::isServiceOnline() const
{
    return m_hiddenService && m_hiddenService->status() == Tor::HiddenService::Online;
}

/* Handle an incoming connection to this service
 *
 * A Protocol::Connection is created to handle this socket. The
 * connection initially has a purpose of Unknown. It times out
 * and automatically closes after ConnectionPrivate::UnknownPurposeTimeout
 * seconds, unless the purpose is changed.
 *
 * If the connection successfully completes authentication,
 * handleIncomingAuthedConnection is called to link it to a ContactUser
 * (if applicable) and set the purpose.
 */
void UserIdentity::onIncomingConnection()
{
    while (m_incomingServer->hasPendingConnections()) {
        QTcpSocket *socket = m_incomingServer->nextPendingConnection();

        /* The localHostname property is used by Connection to determine the
         * server onion hostname that this socket is connected to, which is
         * used by the serverHostname() method.
         */
        socket->setProperty("localHostname", m_hiddenService->hostname());

        qDebug() << "Accepted new incoming connection";
        QSharedPointer<Connection> conn(new Connection(socket, Connection::ServerSide), &QObject::deleteLater);
        Q_ASSERT(socket->parent());

        m_incomingConnections.append(conn);
        Connection *connPtr = conn.data();

        /* When the connection is closed, if it's not claimed, take it out of the
         * incoming connection list and destroy the reference
         */
        connect(connPtr, &Connection::closed, this,
            [this,connPtr]() {
                QSharedPointer<Connection> conn(takeIncomingConnection(connPtr));
                if (conn)
                    qDebug() << "Deleting closed incoming connection that was never claimed by an owner";
            }
        );

        connect(connPtr, &Connection::authenticated, this,
            [this,connPtr](Connection::AuthenticationType type) {
                if (type == Connection::HiddenServiceAuth)
                    handleIncomingAuthedConnection(connPtr);
            }
        );

        emit incomingConnection(connPtr);
    }
}

void UserIdentity::handleIncomingAuthedConnection(Connection *conn)
{
    if (conn->purpose() != Connection::Purpose::Unknown)
        return;

    QString clientName = conn->authenticatedIdentity(Connection::HiddenServiceAuth);
    if (clientName.isEmpty()) {
        BUG() << "Called to handle incoming authed connection without any authed name";
        return;
    }

    ContactUser *user = contacts.lookupHostname(clientName);
    if (!user) {
        // This client can start a contact request, for example. The purpose stays unknown, and the
        // connection will be killed if the purpose isn't changed before the timeout.
        qDebug() << "Have an incoming connection authenticated as unknown client" << clientName;
        return;
    }

    QSharedPointer<Connection> connPtr(takeIncomingConnection(conn));
    if (!connPtr) {
        BUG() << "Called to handle incoming authed connection, but the connection is already out of the incoming list";
        return;
    }

    qDebug() << "Incoming connection authenticated as contact" << user->uniqueID << "with hostname" << clientName;
    user->assignConnection(connPtr);
}

QSharedPointer<Connection> UserIdentity::takeIncomingConnection(Connection *match)
{
    for (auto it = m_incomingConnections.begin(); it != m_incomingConnections.end(); it++) {
        if (it->data() == match) {
            QSharedPointer<Connection> re = *it;
            m_incomingConnections.erase(it);
            return re;
        }
    }

    return QSharedPointer<Connection>();
}
