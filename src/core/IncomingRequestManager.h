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

#ifndef INCOMINGREQUESTMANAGER_H
#define INCOMINGREQUESTMANAGER_H

#include <QObject>
#include <QPointer>
#include <QDateTime>
#include "protocol/ContactRequestServer.h"

class IncomingRequestManager;
class ContactsManager;

class IncomingContactRequest : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IncomingContactRequest)

    Q_PROPERTY(QByteArray hostname READ hostname CONSTANT)
    Q_PROPERTY(QString message READ message CONSTANT)
    Q_PROPERTY(QString nickname READ nickname WRITE setNickname NOTIFY nicknameChanged)
    Q_PROPERTY(bool hasActiveConnection READ hasActiveConnection NOTIFY hasActiveConnectionChanged)
    Q_PROPERTY(QDateTime requestDate READ requestDate CONSTANT)
    Q_PROPERTY(QDateTime lastRequestDate READ lastRequestDate CONSTANT)

public:
    IncomingRequestManager * const manager;

    IncomingContactRequest(IncomingRequestManager *manager, const QByteArray &hostname,
                           ContactRequestServer *connection = 0);

    QByteArray hostname() const { return m_hostname; }

    QByteArray remoteSecret() const { return m_remoteSecret; }
    void setRemoteSecret(const QByteArray &remoteSecret);

    QString message() const { return m_message; }
    void setMessage(const QString &message);

    QString nickname() const { return m_nickname; }
    void setNickname(const QString &nickname);

    bool hasActiveConnection() const { return connection != 0; }
    void setConnection(ContactRequestServer *connection);

    QDateTime requestDate() const { return m_requestDate; }
    QDateTime lastRequestDate() const { return m_lastRequestDate; }

    void renew();

    void load();
    void save();

public slots:
    void accept(ContactUser *user = 0);
    void reject();

signals:
    void nicknameChanged();
    void hasActiveConnectionChanged();

private:
    QPointer<ContactRequestServer> connection;
    QByteArray m_hostname;
    QByteArray m_remoteSecret;
    QString m_message, m_nickname;
    QDateTime m_requestDate, m_lastRequestDate;

    void removeRequest();
};

class IncomingRequestManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IncomingRequestManager)

    Q_PROPERTY(QList<QObject*> requests READ requestObjects NOTIFY requestsChanged)

    friend class IncomingContactRequest;

public:
    ContactsManager * const contacts;

    explicit IncomingRequestManager(ContactsManager *contactsManager);

    QList<QObject*> requestObjects() const;
    QList<IncomingContactRequest*> requests() const { return m_requests; }
    IncomingContactRequest *requestFromHostname(const QByteArray &hostname);

    void loadRequests();

    /* Input from ContactRequestServer */
    void addRequest(const QByteArray &hostname, const QByteArray &connSecret, ContactRequestServer *connection,
                    const QString &nickname, const QString &message);

    /* Blacklist a host for immediate rejection in the future */
    void addRejectedHost(const QByteArray &hostname);
    bool isHostnameRejected(const QByteArray &hostname) const;

signals:
    void requestAdded(IncomingContactRequest *request);
    void requestRemoved(IncomingContactRequest *request);
    void requestsChanged();

private:
    QList<IncomingContactRequest*> m_requests;

    void removeRequest(IncomingContactRequest *request);
};

#endif // INCOMINGREQUESTMANAGER_H
