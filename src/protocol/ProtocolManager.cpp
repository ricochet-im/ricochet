#include "ProtocolManager.h"
#include "ProtocolCommand.h"

ProtocolManager::ProtocolManager(ContactUser *u, const QString &host, quint16 port)
	: QObject(u), user(u), pHost(host), pPort(port), remotePrimary(0)
{
	pPrimary = new ProtocolSocket(this);
	connect(pPrimary, SIGNAL(socketReady()), this, SIGNAL(primaryConnected()));
	connect(pPrimary->socket, SIGNAL(disconnected()), this, SLOT(onPrimaryDisconnected()));
}

void ProtocolManager::setHost(const QString &host)
{
	pHost = host;
}

void ProtocolManager::setPort(quint16 port)
{
	pPort = port;
}

void ProtocolManager::setSecret(const QByteArray &secret)
{
	Q_ASSERT(secret.size() == 16);
	pSecret = secret;

	int size = pSecret.size();
	pSecret.resize(size);

	if (size < 16)
		memset(pSecret.data() + size, 0, 16 - size);
}

bool ProtocolManager::isPrimaryConnected() const
{
	return pPrimary ? pPrimary->isConnected() : false;
}

bool ProtocolManager::isAnyConnected() const
{
	if (isPrimaryConnected())
		return true;

	return false;
}

void ProtocolManager::connectPrimary()
{
	if (pPrimary && (pPrimary->isConnecting() || pPrimary->isConnected()))
		return;

	if (host().isEmpty() || !port())
		return;

	pPrimary->connectToHost(host(), port());
}

void ProtocolManager::addSocket(QTcpSocket *socket, quint8 purpose)
{
	Q_ASSERT(socket->state() == QAbstractSocket::ConnectedState);

	ProtocolSocket *psocket = new ProtocolSocket(socket, this);

	if (purpose == 0x00)
	{
		/* Remote primary connection. */
		if (remotePrimary)
		{
			/* Replaces any older one; a race condition would be possible if the reconnect timeout was
			 * too short, but that shouldn't be an issue. */
			remotePrimary->abort();
			remotePrimary->deleteLater();
		}

		remotePrimary = psocket;

		/* Remote primary connection. To avoid a race condition when both ends establish connections
		 * simultaniously but do not yet know that the connection has been established, we do not
		 * replace a pending connection attempt at this time. If that attempt fails, this connection
		 * may be used as the new local primary. */
		if (!pPrimary || (!pPrimary->isConnecting() && !pPrimary->isConnected()))
		{
			/* TODO lots of fixing */
			if (pPrimary)
			{
				qDebug() << "Replacing unconnected primary socket with incoming socket";
				pPrimary->abort();
				pPrimary->deleteLater();
			}

			pPrimary = psocket;
			connect(pPrimary, SIGNAL(socketReady()), this, SIGNAL(primaryConnected()));
			connect(pPrimary->socket, SIGNAL(disconnected()), this, SIGNAL(primaryDisconnected()));

			emit primaryConnected();
		}
	}
	else
		Q_ASSERT_X(false, "add non-primary socket", "not implemented");
}

void ProtocolManager::onPrimaryDisconnected()
{

}
