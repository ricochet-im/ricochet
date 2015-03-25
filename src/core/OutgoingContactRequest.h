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

#ifndef OUTGOINGCONTACTREQUEST_H
#define OUTGOINGCONTACTREQUEST_H

#include <QObject>
#include "utils/Settings.h"

class ContactUser;
class ContactRequestClient;

namespace Protocol {
    class Connection;
}

class OutgoingContactRequest : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutgoingContactRequest)
    Q_ENUMS(Status)

    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString myNickname READ myNickname CONSTANT)
    Q_PROPERTY(QString message READ message CONSTANT)
    Q_PROPERTY(QString rejectMessage READ rejectMessage NOTIFY rejected)
#ifndef PROTOCOL_NEW
    Q_PROPERTY(bool isConnected READ isConnected NOTIFY connectedChanged)
#endif

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

    static OutgoingContactRequest *createNewRequest(ContactUser *user, const QString &myNickname, const QString &message);

    ContactUser * const user;

    OutgoingContactRequest(ContactUser *user);
    virtual ~OutgoingContactRequest();

    QString myNickname() const;
    QString message() const;
    Status status() const;
    QString rejectMessage() const;

#ifndef PROTOCOL_NEW
    bool isConnected() const;
    ContactRequestClient *client() const { return m_client; }
#endif

public slots:
    void accept();
    void reject(bool error = false, const QString &reason = QString());
    void cancel();

#ifdef PROTOCOL_NEW
    void sendRequest(Protocol::Connection *connection);
#endif

signals:
    void statusChanged(int newStatus, int oldStatus);
    void accepted();
    void rejected(const QString &reason);
    void removed();

#ifndef PROTOCOL_NEW
    void connectedChanged();
#endif

private slots:
#ifdef PROTOCOL_NEW
    void requestStatusChanged(int status, const QString &message);
#else
    void requestRejected(int reason);
    void requestAcknowledged();

    void startConnection();
#endif

private:
#ifndef PROTOCOL_NEW
    ContactRequestClient *m_client;
#endif
    SettingsObject *m_settings;

    void setStatus(Status newStatus);
    void removeRequest();
    void attemptAutoAccept();
};

#endif // OUTGOINGCONTACTREQUEST_H
