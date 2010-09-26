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

#ifndef CONTACTREQUESTSERVER_H
#define CONTACTREQUESTSERVER_H

#include <QObject>

class QTcpSocket;
class ContactUser;
class UserIdentity;

class ContactRequestServer : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactRequestServer)

public:
    UserIdentity * const identity;

    explicit ContactRequestServer(UserIdentity *identity, QTcpSocket *socket);

    void sendAccept(ContactUser *user);
    void sendRejection();

    void close();

private slots:
    void socketReadable();
    void socketDisconnected();

private:
    QTcpSocket * const socket;
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
