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

#ifndef CONTACTREQUESTSERVER_H
#define CONTACTREQUESTSERVER_H

#include <QObject>
#include <QTimer>

class QTcpSocket;
class ContactUser;
class UserIdentity;

/* Incoming connection with a purpose of 0x80 (contact request) */
class ContactRequestServer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactRequestServer)

public:
    UserIdentity * const identity;

    explicit ContactRequestServer(UserIdentity *identity, QTcpSocket *socket);

public slots:
    void sendAccept(ContactUser *user);
    void sendRejection();
    void close();

private slots:
    void socketReadable();
    void socketDisconnected();

private:
    QTcpSocket * const socket;
    QTimer *timeout;
    QByteArray cookie;

    enum
    {
        WaitRequest,
        WaitResponse,
        SentResponse
    } state;

    void sendCookie();
    bool sendResponse(uchar response);

    void handleRequest(const QByteArray &data);
};

#endif // CONTACTREQUESTSERVER_H
