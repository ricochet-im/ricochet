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

#ifndef OUTGOINGCONTACTREQUEST_H
#define OUTGOINGCONTACTREQUEST_H

#include <QObject>

class ContactUser;
class ContactRequestClient;

class OutgoingContactRequest : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutgoingContactRequest)

public:
    enum Status
    {
        Pending,
        Acknowledged,
        Accepted,
        Error,
        Rejected,
        FirstResult = Accepted
    };

    static OutgoingContactRequest *requestForUser(ContactUser *user);
    static OutgoingContactRequest *createNewRequest(ContactUser *user, const QString &myNickname, const QString &message);

    ContactUser * const user;

    QString myNickname() const;
    QString message() const;
    Status status() const;
    QString rejectMessage() const;

    ContactRequestClient *client() const { return m_client; }

public slots:
    void accept();
    void reject(bool error = false, const QString &reason = QString());

signals:
    void statusChanged(int newStatus, int oldStatus);
    void accepted();
    void rejected(const QString &reason);

private slots:
    void requestRejected(int reason);

    void startConnection();

private:
    ContactRequestClient *m_client;

    OutgoingContactRequest(ContactUser *user);

    void setStatus(Status newStatus);
    void removeRequest();
    void attemptAutoAccept();
};

#endif // OUTGOINGCONTACTREQUEST_H
