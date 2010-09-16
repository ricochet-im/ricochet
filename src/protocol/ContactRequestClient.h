/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
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
