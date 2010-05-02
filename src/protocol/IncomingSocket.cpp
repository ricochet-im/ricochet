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

#include "IncomingSocket.h"
#include "ProtocolManager.h"
#include "core/ContactsManager.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QDateTime>
#include <QtDebug>

IncomingSocket::IncomingSocket(QObject *parent)
	: QObject(parent), server(new QTcpServer(this))
{
	connect(server, SIGNAL(newConnection()), this, SLOT(incomingConnection()));
}

bool IncomingSocket::listen(const QHostAddress &address, quint16 port)
{
	if (server->isListening())
		server->close();

	return server->listen(address, port);
}

QString IncomingSocket::errorString() const
{
	return server->errorString();
}

QHostAddress IncomingSocket::serverAddress() const
{
	return server->serverAddress();
}

quint16 IncomingSocket::serverPort() const
{
	return server->serverPort();
}

void IncomingSocket::incomingConnection()
{
	while (server->hasPendingConnections())
	{
		QTcpSocket *conn = server->nextPendingConnection();
		connect(conn, SIGNAL(readyRead()), this, SLOT(readSocket()));
		connect(conn, SIGNAL(disconnected()), this, SLOT(removeSocket()));

		conn->setParent(this);
		conn->setProperty("startTime", QDateTime::currentDateTime());
		pendingSockets.append(conn);

		if (!expireTimer.isActive())
			expireTimer.start(10000, this);
	}
}

void IncomingSocket::timerEvent(QTimerEvent *)
{
	QDateTime now = QDateTime::currentDateTime();

	for (int i = 0; i < pendingSockets.size(); ++i)
	{
		QDateTime started = pendingSockets[i]->property("startTime").toDateTime();
		if (started.secsTo(now) >= 30)
		{
			/* time is up. */
			removeSocket(pendingSockets[i]);
			--i;
		}
	}

	if (pendingSockets.isEmpty())
		expireTimer.stop();
}

void IncomingSocket::readSocket()
{
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
	if (!socket)
		return;

	/* 0x49 0x4D [1*version] [16*cookie] [1*purpose] */
	qint64 available = socket->bytesAvailable();

	if (available < 3)
		return;

	/* Peek for the intro bytes and the version */
	char intro[3];
	qint64 re = socket->peek(intro, 3);
	if (re < 3)
		return;

	if (intro[0] != 0x49 || intro[1] != 0x4D)
	{
		qDebug() << "Connection authentication failed: incorrect introduction sequence";
		removeSocket(socket);
		return;
	}

	if (intro[2] != protocolVersion)
	{
		qDebug() << "Connection authentication failed: protocol version mismatch - " << hex
				<< (int)intro[2];
		removeSocket(socket);
		return;
	}

	/* Wait until the full introduction is available */
	if (available < 20)
		return;

	QByteArray data = socket->read(20);
	Q_ASSERT(data.size() == 20);

	ContactUser *user = contactsManager->lookupSecret(data.mid(3, 16));

	if (!user)
	{
		qDebug() << "Connection authentication failed: no match for secret";
		removeSocket(socket);
		return;
	}

	qDebug() << "Connection authentication successful for" << user->nickname()
			<< "purpose" << hex << (int)data[19];

	char response = 0x01;
	socket->write(&response, 1);

	pendingSockets.removeOne(socket);
	socket->disconnect(this);

	/* The protocolmanager also takes ownership */
	user->conn()->addSocket(socket, data[19]);
	Q_ASSERT(socket->parent() != this);
}

void IncomingSocket::removeSocket(QTcpSocket *socket)
{
	if (!socket)
	{
		socket = qobject_cast<QTcpSocket*>(sender());
		if (!socket)
			return;
	}

	qDebug() << "Disconnecting pending socket";

	pendingSockets.removeOne(socket);

	socket->disconnect(this);
	socket->close();
	socket->deleteLater();
}
