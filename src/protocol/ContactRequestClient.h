/* Torsion - http://torsionim.org/
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

#ifndef CONTACTREQUESTCLIENT_H
#define CONTACTREQUESTCLIENT_H

#include <QObject>

class ContactUser;

class ContactRequestClient : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactRequestClient)

public:
    ContactUser * const user;

    enum Response
    {
        NoResponse,
        Acknowledged,
        Accepted,
        Rejected,
        Error
    };

    explicit ContactRequestClient(ContactUser *user);

    void close();

    QString message() const { return m_message; }
    void setMessage(const QString &message);

    QString myNickname() const { return m_mynick; }
    void setMyNickname(const QString &nick);

    Response response() const { return m_response; }

public slots:
    void sendRequest();

signals:
    void acknowledged();
    void accepted();
    /* reason is the raw code sent by the peer, not a Response. */
    void rejected(int reason);

private slots:
    void socketConnected();
    void socketReadable();

    void spawnReconnect();

private:
    class QTcpSocket *socket;
    QString m_message, m_mynick;
    int connectAttempts;
    Response m_response;

    enum
    {
        NotConnected,
        Reconnecting,
        WaitConnect,
        WaitCookie,
        WaitAck,
        WaitResponse
    } state;

    bool buildRequestData(QByteArray cookie);
    bool handleResponse();
};

#endif // CONTACTREQUESTCLIENT_H
