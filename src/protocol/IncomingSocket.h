/* Torsion - http://github.com/special/torsion
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

#ifndef INCOMINGSOCKET_H
#define INCOMINGSOCKET_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QBasicTimer>

class QTcpServer;
class QTcpSocket;

class IncomingSocket : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IncomingSocket)

public:
    explicit IncomingSocket(QObject *parent = 0);

    bool listen(const QHostAddress &address, quint16 port);
    QString errorString() const;

    QHostAddress serverAddress() const;
    quint16 serverPort() const;

    /* The intro based on supported protocols; anything after the purpose is left to the caller. */
    static QByteArray introData(uchar purpose);

private slots:
    void incomingConnection();

    void readSocket();
    void removeSocket(QTcpSocket *socket = 0);

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    QTcpServer *server;
    QList<QTcpSocket*> pendingSockets;
    QBasicTimer expireTimer;

    bool handleVersion(QTcpSocket *socket);
    void handleIntro(QTcpSocket *socket, uchar version);
};

#endif // INCOMINGSOCKET_H
