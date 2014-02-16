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
#include "ContactUser.h"
#include "UserIdentity.h"
#include "ContactsManager.h"
#include "utils/SecureRNG.h"
#include "protocol/GetSecretCommand.h"
#include "protocol/ChatMessageCommand.h"
#include "core/ContactIDValidator.h"
#include "core/OutgoingContactRequest.h"
#include <QPixmapCache>
#include <QtDebug>
#include <QBuffer>
#include <QDateTime>

ContactUser::ContactUser(UserIdentity *ident, int id, QObject *parent)
    : QObject(parent), identity(ident), uniqueID(id), m_lastReceivedChatID(0),
      m_contactRequest(0)
{
    Q_ASSERT(uniqueID >= 0);

    loadSettings();

    m_conn = new ProtocolSocket(this);

    QByteArray remoteSecret = readSetting("remoteSecret").toByteArray();
    //if (!remoteSecret.isNull())
      //  m_conn->setSecret(remoteSecret);

    connect(m_conn, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(m_conn, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

    loadContactRequest();
    updateStatus();
}

void ContactUser::loadSettings()
{
    config->beginGroup(QLatin1String("contacts/") + QString::number(uniqueID));

    m_nickname = config->value("nickname", uniqueID).toString();

    config->endGroup();
}

void ContactUser::loadContactRequest()
{
    if (m_contactRequest)
        return;

    if (!readSetting("request/status").isNull()) {
        m_contactRequest = new OutgoingContactRequest(this);
        connect(m_contactRequest, SIGNAL(statusChanged(int,int)), SLOT(updateStatus()));
        connect(m_contactRequest, SIGNAL(removed()), SLOT(requestRemoved()));
        updateStatus();
    }
}

QVariant ContactUser::readSetting(const QString &key, const QVariant &defaultValue) const
{
    return config->value(QString::fromLatin1("contacts/%1/%2").arg(uniqueID).arg(key), defaultValue);
}

void ContactUser::writeSetting(const QString &key, const QVariant &value)
{
    config->setValue(QString::fromLatin1("contacts/%1/%2").arg(uniqueID).arg(key), value);
}

void ContactUser::removeSetting(const QString &key)
{
    config->remove(QString::fromLatin1("contacts/%1/%2").arg(uniqueID).arg(key));
}

ContactUser *ContactUser::addNewContact(UserIdentity *identity, int id)
{
    ContactUser *user = new ContactUser(identity, id);
    user->writeSetting("whenCreated", QDateTime::currentDateTime());

    /* Generate the local secret and set it */
    user->writeSetting("localSecret", SecureRNG::random(16));

    return user;
}

void ContactUser::updateStatus()
{
    Status newStatus;
    if (!readSetting("request/status").isNull())
        newStatus = RequestPending;
    else
        newStatus = m_conn->isConnected() ? Online : Offline;

    if (newStatus != m_status)
    {
        m_status = newStatus;
        emit statusChanged();
    }

    // XXX Start connecting if offline and not already trying
    // important for incoming requests accepted too
}

void ContactUser::onConnected()
{
    writeSetting("lastConnected", QDateTime::currentDateTime());

    if (m_contactRequest)
    {
        qDebug() << "Implicitly accepting outgoing contact request for" << uniqueID << "from primary connection";

        m_contactRequest->accept();
        updateStatus();
        Q_ASSERT(status() != RequestPending);
    }

    if (readSetting("remoteSecret").isNull())
    {
        qDebug() << "Requesting remote secret from user" << uniqueID;
        GetSecretCommand *command = new GetSecretCommand(this);
        command->send(conn());
    }

    updateStatus();
    emit connected();
}

void ContactUser::onDisconnected()
{
    writeSetting("lastConnected", QDateTime::currentDateTime());

    updateStatus();
    emit disconnected();
}

void ContactUser::setNickname(const QString &nickname)
{
    if (m_nickname == nickname)
        return;

    /* non-critical, just a safety net for UI checks */
    Q_ASSERT(!identity->contacts.lookupNickname(nickname));

    m_nickname = nickname;

    writeSetting("nickname", nickname);
    emit nicknameChanged();
}

QString ContactUser::hostname() const
{
    return readSetting("hostname").toString();
}

quint16 ContactUser::port() const
{
    return (quint16)readSetting("port", 9878).toUInt();
}

QString ContactUser::contactID() const
{
    return ContactIDValidator::idFromHostname(hostname());
}

void ContactUser::setHostname(const QString &hostname)
{
    QString fh = hostname;

    if (!hostname.endsWith(QLatin1String(".onion")))
        fh.append(QLatin1String(".onion"));

    writeSetting(QLatin1String("hostname"), fh);
    //conn()->setHost(fh);
}

void ContactUser::setAvatar(QImage image)
{
    if (image.width() > 160 || image.height() > 160)
        image = image.scaled(QSize(160, 160), Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (!image.isNull())
    {
        QBuffer buffer;
        buffer.open(QBuffer::ReadWrite);
        if (image.save(&buffer, "jpeg", 90))
            writeSetting("avatar", buffer.buffer());
        else
            image = QImage();
    }

    if (image.isNull())
        removeSetting("avatar");
}

void ContactUser::deleteContact()
{
    /* Anything that uses ContactUser is required to either respond to the contactDeleted signal
     * synchronously, or make use of QWeakPointer. */

    qDebug() << "Deleting contact" << uniqueID;

    if (m_contactRequest)
    {
        qDebug() << "Cancelling request associated with contact to be deleted";
        m_contactRequest->cancel();
        m_contactRequest->deleteLater();
    }

    emit contactDeleted(this);

    m_conn->disconnect();
    delete m_conn;
    m_conn = 0;

    config->remove(QLatin1String("contacts/") + QString::number(uniqueID));

    deleteLater();
}

void ContactUser::requestRemoved()
{
    if (m_contactRequest) {
        m_contactRequest->deleteLater();
        m_contactRequest = 0;
        updateStatus();
    }
}

void ContactUser::incomingProtocolSocket(QTcpSocket *socket)
{
    /* Per protocol.txt, to resolve connection races:
     *
     * - If the remote peer establishes a command connection prior to
     *   sending authentication data for an outgoing attempt, the outgoing
     *   attempt is aborted and the peer's connection is used.
     *
     * - If the existing connection is more than 30 seconds old, measured
     *   from the time authentication is sent, it is replaced and closed.
     *
     * - Otherwise, both peers close the connection for which the server's
     *   onion-formatted hostname is considered less by a strcmp function.
     */
    // XXX implement the above
    conn()->setSocket(socket);
}

