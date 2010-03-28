#include "ProtocolManager.h"
#include "ProtocolCommand.h"
#include <QTcpSocket>
#include <QtEndian>

ProtocolManager::ProtocolManager(const QString &host, quint16 port, QObject *parent)
	: QObject(parent), primarySocket(0), pHost(host), pPort(port)
{
}

void ProtocolManager::setHost(const QString &host)
{
	pHost = host;
}

void ProtocolManager::setPort(quint16 port)
{
	pPort = port;
}

bool ProtocolManager::isPrimaryConnected() const
{
	return primarySocket ? (primarySocket->state() == QAbstractSocket::ConnectedState) : false;
}

bool ProtocolManager::isAnyConnected() const
{
	if (isPrimaryConnected())
		return true;

	for (QList<QTcpSocket*>::ConstIterator it = socketPool.begin(); it != socketPool.end(); ++it)
		if ((*it)->state() == QAbstractSocket::ConnectedState)
			return true;

	return false;
}

void ProtocolManager::connectPrimary()
{
	if (primarySocket && primarySocket->state() != QAbstractSocket::UnconnectedState)
		return;

	if (!primarySocket)
	{
		primarySocket = new QTcpSocket(this);
		connect(primarySocket, SIGNAL(connected()), this, SLOT(socketConnected()));
		connect(primarySocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
		connect(primarySocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
				SLOT(socketError(QAbstractSocket::SocketError)));
		connect(primarySocket, SIGNAL(readyRead()), this, SLOT(socketReadable()));
	}

	qDebug() << "Attempting to connect primary socket to" << host() << "on port" << port();
	primarySocket->connectToHost(host(), port());
}

void ProtocolManager::connectAnother()
{
	qFatal("ProtocolManager::ConnectAnother - not implemented");
}

void ProtocolManager::addSocket(QTcpSocket *socket, quint8 purpose)
{
	Q_ASSERT(socket->state() == QAbstractSocket::ConnectedState);

	socket->setParent(this);

	if (purpose == 0x00)
	{
		/* Primary, serial connection. This is not required to be the same connection on
		 * both ends, but you cannot use a connection as the primary when the other end
		 * does not consider it to be one. */
		if (!isPrimaryConnected())
		{
			if (primarySocket)
			{
				primarySocket->abort();
				primarySocket->deleteLater();
			}

			primarySocket = socket;
		}
	}
	else
	{
		/* Nothing yet */
		qFatal("Non-primary sockets are not implemented");
	}

	socketConnected(socket);
}

quint16 ProtocolManager::getIdentifier() const
{
	/* There is a corner case for the very unlucky where the RNG will take a very long time
	 * to find an available ID. This could be considered a BUG. */
	if (pendingCommands.size() >= 50000)
		return 0;

	quint16 re;
	do
	{
		re = (qrand() % 65535) + 1;
	} while (pendingCommands.contains(re));

	return re;
}

void ProtocolManager::sendCommand(ProtocolCommand *command, bool ordered)
{
	Q_ASSERT(!pendingCommands.contains(command->identifier()));

	pendingCommands.insert(command->identifier(), command);

	if (ordered)
	{
		if (!isPrimaryConnected())
		{
			commandQueue.append(command);
			return;
		}

		Q_ASSERT(commandQueue.isEmpty());
		primarySocket->write(command->commandBuffer);
	}
	else
	{
		qFatal("Not implemented");
	}
}

void ProtocolManager::socketConnected(QTcpSocket *socket)
{
	if (!socket)
	{
		socket = qobject_cast<QTcpSocket*>(sender());
		if (!socket)
		{
			Q_ASSERT_X(false, "ProtocolManager", "socketConnected signal from an unexpected source");
			return;
		}
	}

	quint8 purpose;
	if (socket == primarySocket)
		purpose = 0x00;
	else
		purpose = 0xff;

	/* Introduction; 0x49 0x4D [1*version] [16*cookie] [1*purpose] */
	QByteArray intro;
	intro.resize(20);

	intro[0] = 0x49;
	intro[1] = 0x4D;
	intro[2] = protocolVersion;
	intro[19] = purpose;

	socket->setProperty("authPending", true);
	qint64 re = socket->write(intro);
	Q_ASSERT(re == intro.size());
}


void ProtocolManager::socketAuthenticated(QTcpSocket *socket)
{
	if (socket == primarySocket)
	{
		qDebug() << "Primary socket connected";

		while (!commandQueue.isEmpty())
			primarySocket->write(commandQueue.takeFirst()->commandBuffer);

		emit primaryConnected();
	}
}

void ProtocolManager::socketDisconnected()
{
	qDebug() << "Socket disconnected";
}

void ProtocolManager::socketError(QAbstractSocket::SocketError error)
{
	qDebug() << "Socket error:" << error;
}

void ProtocolManager::socketReadable()
{
	qDebug() << "Socket readable";

	QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
	if (!socket)
		return;

	qint64 available = socket->bytesAvailable();

	if (available && socket->property("authPending").toBool())
	{
		char reply;
		qint64 re = socket->read(&reply, 1);
		Q_ASSERT(re);

		if (reply != 0x01)
		{
			socket->close();
			return;
		}

		socket->setProperty("authPending", QVariant());
		socketAuthenticated(socket);

		available--;
	}

	while (available >= 6)
	{
		quint16 msgLength;
		if (socket->peek(reinterpret_cast<char*>(&msgLength), sizeof(msgLength)) < 2)
			return;

		msgLength = qFromBigEndian(msgLength);
		if (!msgLength)
			qFatal("Unbuffered protocol replies are not implemented");

		/* Message length is one more than the actual data length, and does not include the header. */
		msgLength--;
		if ((available - 6) < msgLength)
			break;

		QByteArray data;
		data.resize(msgLength + 6);

		qint64 re = socket->read(data.data(), msgLength + 6);
		Q_ASSERT(re == msgLength + 6);

		callCommand(data[2], data[3],
					qFromBigEndian<quint16>(reinterpret_cast<const uchar*>(data.constData())+4),
					reinterpret_cast<const uchar*>(data.constData()), msgLength);

		available -= msgLength + 6;
	}
}
