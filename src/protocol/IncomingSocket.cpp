#include "IncomingSocket.h"
#include "ProtocolManager.h"
#include <QTcpServer>
#include <QTcpSocket>
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

void IncomingSocket::incomingConnection()
{
	while (server->hasPendingConnections())
	{
		QTcpSocket *conn = server->nextPendingConnection();
		connect(conn, SIGNAL(readyRead()), this, SLOT(readSocket()));
		connect(conn, SIGNAL(disconnected()), this, SLOT(removeSocket()));

		conn->setParent(this);
		pendingSockets.append(conn);
	}
}

void IncomingSocket::readSocket()
{
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
	if (!socket)
		return;

	/* 0x49 0x4D [1*version] [16*cookie] - expecting 19 bytes, check after 3 for version */
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
	if (available < 19)
		return;

	qDebug() << "Authentication data is available for a socket!";
	removeSocket(socket);
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
