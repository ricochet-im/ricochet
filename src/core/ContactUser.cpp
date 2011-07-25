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
#include "utils/DateUtil.h"
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
    : QObject(parent), identity(ident), uniqueID(id), m_lastReceivedChatID(0)
{
    Q_ASSERT(uniqueID >= 0);

    loadSettings();

    /* Connection */
    QString host = readSetting("hostname").toString();
    quint16 port = (quint16)readSetting("port", 80).toUInt();
    m_conn = new ProtocolManager(this, host, port);

    QByteArray remoteSecret = readSetting("remoteSecret").toByteArray();
    if (!remoteSecret.isNull())
        m_conn->setSecret(remoteSecret);

    connect(m_conn, SIGNAL(primaryConnected()), this, SLOT(onConnected()));
    connect(m_conn, SIGNAL(primaryDisconnected()), this, SLOT(onDisconnected()));

    updateStatus();

    /* Outgoing request */
    if (!readSetting("request/status").isNull())
    {
        /* Used to initialize the request on startup for existing requests */
        OutgoingContactRequest *request = OutgoingContactRequest::requestForUser(this);
        Q_ASSERT(request);
        Q_UNUSED(request);
    }
}

void ContactUser::loadSettings()
{
    config->beginGroup(QLatin1String("contacts/") + QString::number(uniqueID));

    m_nickname = config->value("nickname", uniqueID).toString();

    config->endGroup();
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
        newStatus = m_conn->isPrimaryConnected() ? Online : Offline;

    if (newStatus != m_status)
    {
        m_status = newStatus;
        emit statusChanged();
    }
}

void ContactUser::onConnected()
{
    writeSetting("lastConnected", QDateTime::currentDateTime());

    if (isContactRequest())
    {
        qDebug() << "Implicitly accepting outgoing contact request for" << uniqueID << "from primary connection";

        OutgoingContactRequest *request = OutgoingContactRequest::requestForUser(this);
        Q_ASSERT(request);
        request->accept();
        Q_ASSERT(!isContactRequest());
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
    conn()->setHost(fh);
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

    if (isContactRequest())
    {
        OutgoingContactRequest *request = OutgoingContactRequest::requestForUser(this);
        if (request)
        {
            qDebug() << "Cancelling request associated with contact to be deleted";
            request->cancel();
            delete request;
        }
    }

    emit contactDeleted(this);

    m_conn->disconnectAll();
    delete m_conn;
    m_conn = 0;

    config->remove(QLatin1String("contacts/") + QString::number(uniqueID));

    deleteLater();
}

QString ContactUser::statusString(Status status)
{
    switch (status)
    {
    case Online: return tr("Online");
    case Offline: return tr("Offline");
    case RequestPending: return tr("Pending Requests");
    default: return QString();
    }
}

void ContactUser::sendChatMessage(const QString &text)
{
    QDateTime when = QDateTime::currentDateTime();

    if (isConnected())
    {
        ChatMessageCommand *command = new ChatMessageCommand;
        command->send(conn(), when, text, m_lastReceivedChatID);
    }
    else
        qCritical("XXX-UI sendChatMessage offline messaging is not implemented");
    /*else
    {
        int n = addOfflineMessage(when, text);
        addChatMessage(NULL, (quint16)-1, when, text);
        changeBlockIdentifier(makeBlockIdentifier(LocalUserMessage, (quint16)-1), makeBlockIdentifier(OfflineMessage, n));
    }*/
}
