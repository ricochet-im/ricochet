#include "ProtocolManager.h"
#include "ProtocolCommand.h"
#include <QTcpSocket>

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
	if (isPrimaryConnected() || primarySocket->state() != QAbstractSocket::UnconnectedState)
		return;

	if (!primarySocket)
	{
		primarySocket = new QTcpSocket(this);
		connect(primarySocket, SIGNAL(connected()), this, SLOT(socketConnected()));
		connect(primarySocket, SIGNAL(readyRead()), this, SLOT(socketReadable()));
	}

	qDebug() << "Attempting to connect primary socket to" << host() << "on port" << port();
	primarySocket->connectToHost(host(), port());
}

void ProtocolManager::connectAnother()
{
	qFatal("ProtocolManager::ConnectAnother - not implemented");
}

void ProtocolManager::socketConnected()
{
	QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
	if (!socket)
	{
		Q_ASSERT_X(false, "ProtocolManager", "socketConnected signal from an unexpected source");
		return;
	}

	if (socket == primarySocket)
		emit primaryConnected();
}

void ProtocolManager::socketReadable()
{
	qDebug() << "Socket readable";
}

quint16 ProtocolManager::getIdentifier() const
{
	quint16 re;
	do
	{
		re = (qrand() % 65535) + 1;
	} while (pendingCommands.contains(re));

	return re;
}
