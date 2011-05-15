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

#ifndef CONTACTUSER_H
#define CONTACTUSER_H

#include "main.h"
#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QHash>
#include <QPixmapCache>
#include <QMetaType>
#include <QVariant>
#include "protocol/ProtocolManager.h"

class UserIdentity;
struct ChatMessageData;

/* Represents a user on the contact list.
 * All persistent uses of a ContactUser instance must either connect to the
 * contactDeleted() signal, or use a QWeakPointer to track deletion. A ContactUser
 * can be removed at essentially any time. */

class ContactUser : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactUser)
    Q_ENUMS(Status)

    Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
    Q_PROPERTY(QString contactID READ contactID CONSTANT)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    friend class ContactsManager;
    friend class ChatMessageCommand;

public:
    enum Status
    {
        Online,
        Offline,
        RequestPending
    };

    UserIdentity * const identity;
    const int uniqueID;

    explicit ContactUser(UserIdentity *identity, int uniqueID, QObject *parent = 0);

    ProtocolManager *conn() const { return m_conn; }
    bool isConnected() const { return status() == Online; }
    bool isConnectable() const { return m_conn->isConnectable(); }

    bool isContactRequest() const { return status() == RequestPending; }

    const QString &nickname() const { return m_nickname; }
    /* Hostname is in the onion hostname format, i.e. it ends with .onion */
    QString hostname() const;
    /* Contact ID in the @Torsion format */
    QString contactID() const;
    QPixmap avatar(AvatarSize size);

    Status status() const { return m_status; }
    QString statusString() const { return statusString(m_status); }
    static QString statusString(Status status);

    QVariant readSetting(const QString &key, const QVariant &defaultValue = QVariant()) const;
    QVariant readSetting(const char *key, const QVariant &defaultValue = QVariant()) const
    {
        return readSetting(QLatin1String(key), defaultValue);
    }

    void writeSetting(const QString &key, const QVariant &value);
    void writeSetting(const char *key, const QVariant &value)
    {
        writeSetting(QLatin1String(key), value);
    }

    void removeSetting(const QString &key);
    void removeSetting(const char *key)
    {
        removeSetting(QLatin1String(key));
    }

    void deleteContact();

    Q_INVOKABLE void sendChatMessage(const QString &text);

public slots:
    void setNickname(const QString &nickname);
    void setHostname(const QString &hostname);
    void setAvatar(QImage image);

signals:
    void statusChanged();
    void connected();
    void disconnected();

    void nicknameChanged();
    void contactDeleted(ContactUser *user);

    void incomingChatMessage(const ChatMessageData &message);
    void outgoingChatMessage(const ChatMessageData &message, ChatMessageCommand *command);

private slots:
    void onConnected();
    void onDisconnected();

private:
    ProtocolManager *m_conn;
    QString m_nickname;
    Status m_status;
    quint16 m_lastReceivedChatID;

    /* See ContactsManager::addContact */
    static ContactUser *addNewContact(UserIdentity *identity, int id);

    void loadSettings();
    QString avatarCacheKey(AvatarSize size);
};

Q_DECLARE_METATYPE(ContactUser*)

#endif // CONTACTUSER_H
