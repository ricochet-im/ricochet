/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
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
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

#ifndef INCOMINGREQUESTMANAGER_H
#define INCOMINGREQUESTMANAGER_H

#include <QObject>
#include <QWeakPointer>
#include <QDateTime>
#include "protocol/ContactRequestServer.h"

class IncomingRequestManager;
class ContactsManager;

class IncomingContactRequest : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IncomingContactRequest)

public:
    IncomingRequestManager * const manager;
    const QByteArray hostname;

    IncomingContactRequest(IncomingRequestManager *manager, const QByteArray &hostname,
                           ContactRequestServer *connection = 0);

    QByteArray remoteSecret() const { return m_remoteSecret; }
    void setRemoteSecret(const QByteArray &remoteSecret);

    QString message() const { return m_message; }
    void setMessage(const QString &message);

    QString nickname() const { return m_nickname; }

    bool hasActiveConnection() const { return connection != 0; }
    void setConnection(ContactRequestServer *connection);

    QDateTime requestDate() const { return m_requestDate; }
    QDateTime lastRequestDate() const { return m_lastRequestDate; }

    void renew();

    void load();
    void save();

public slots:
    void setNickname(const QString &nickname);

    void accept(ContactUser *user = 0);
    void reject();

private:
    QWeakPointer<ContactRequestServer> connection;
    QByteArray m_remoteSecret;
    QString m_message, m_nickname;
    QDateTime m_requestDate, m_lastRequestDate;

    void removeRequest();
};

class IncomingRequestManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IncomingRequestManager)

    friend class IncomingContactRequest;

public:
    ContactsManager * const contacts;

    explicit IncomingRequestManager(ContactsManager *contactsManager);

    QList<IncomingContactRequest*> requests() const { return m_requests; }
    IncomingContactRequest *requestFromHostname(const QByteArray &hostname);

    void loadRequests();

    /* Input from ContactRequestServer */
    void addRequest(const QByteArray &hostname, const QByteArray &connSecret, ContactRequestServer *connection,
                    const QString &nickname, const QString &message);

    /* Blacklist a host for immediate rejection in the future */
    void addRejectedHost(const QByteArray &hostname);
    bool isHostnameRejected(const QByteArray &hostname) const;

    QStringList rejectedHosts() const;

signals:
    void requestAdded(IncomingContactRequest *request);
    void requestRemoved(IncomingContactRequest *request);

private:
    QList<IncomingContactRequest*> m_requests;

    void removeRequest(IncomingContactRequest *request);
};

#endif // INCOMINGREQUESTMANAGER_H
