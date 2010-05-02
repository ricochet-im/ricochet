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

#include "ProtocolManager.h"
#include "ProtocolCommand.h"
#include "tor/TorControlManager.h"
#include <QTimer>

ProtocolManager::ProtocolManager(ContactUser *u, const QString &host, quint16 port)
	: QObject(u), user(u), pPrimary(0), remotePrimary(0), pHost(host), pPort(port), connectAttempts(0)
{
	setPrimary(new ProtocolSocket(this));
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

void ProtocolManager::setPrimary(ProtocolSocket *newPrimary)
{
	if (pPrimary == newPrimary)
		return;

	if (pPrimary)
	{
		/* TODO: copy pending commands and such */
		qDebug() << "Replacing primary socket with a new socket";
		pPrimary->abort();
		pPrimary->disconnect(this);
		pPrimary->deleteLater();
	}

	pPrimary = newPrimary;
	connect(pPrimary, SIGNAL(socketReady()), this, SLOT(onPrimaryConnected()));
	connect(pPrimary, SIGNAL(disconnected()), this, SLOT(onPrimaryDisconnected()));
	connect(pPrimary, SIGNAL(connectFailed()), this, SLOT(spawnReconnect()));

	if (pPrimary->isConnected())
		emit primaryConnected();
}

void ProtocolManager::connectPrimary()
{
	Q_ASSERT(pPrimary);

	if (pPrimary->isConnecting() || pPrimary->isConnected())
		return;

	/* The contact is responsible for triggering connection should host or port
	 * change. The tor manager check is safe because ContactsManager will always
	 * spawn a connection for all contacts when the socksReady state changes. */
	if (host().isEmpty() || !port() || !torManager->isSocksReady())
		return;

	pPrimary->connectToHost(host(), port());
}

void ProtocolManager::spawnReconnect()
{
	/* See if any other connections can be used as the new primary */
	if (remotePrimary && remotePrimary != pPrimary && remotePrimary->isConnected())
	{
		setPrimary(remotePrimary);
		return;
	}

	/* TODO: Outgoing auxiliary connections with no owner can be repurposed as a new primary connection */

	if (!torManager->isSocksReady())
	{
		/* See the note above; connectPrimary is triggered when socks becomes ready,
		 * and there is no point in connecting prior to that. We can simply abort. */
		qDebug() << "Waiting for SOCKS to become ready for primary connection";
		connectAttempts = 0;
		return;
	}

	connectAttempts++;

	int delay = 0;

	/* For the first 6 attempts, scale linearly at a delay of 45 seconds
	 * For each following attempt, add 90 seconds to a maximum of 15 minutes.
	 * These numbers are completely arbitrary. */
	if (connectAttempts <= 6)
		delay = connectAttempts * 45;
	else
		delay = qMin((6 * 45) + ((connectAttempts - 6) * 90), 900);

	qDebug() << "Spawning reconnection to" << user->uniqueID << "with a delay of" << delay << "seconds";

	QTimer::singleShot(delay * 1000, this, SLOT(connectPrimary()));
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

		/* If there is no existing local primary connection and no active attempt, use this as the local primary.
		 * If there is an active attempt, wait 30 seconds and abort it if it has not completed in that time. That
		 * will then trigger logic that will use the remote primary as the new local primary. */
		if (pPrimary && pPrimary->isConnecting())
		{
			qDebug() << "Waiting 30 seconds for an existing connection attempt before using the remote primary";
			QTimer::singleShot(30000, pPrimary, SLOT(abortConnectionAttempt()));
		}
		else if (!pPrimary || !pPrimary->isConnected())
			setPrimary(remotePrimary);
	}
	else
		Q_ASSERT_X(false, "add non-primary socket", "not implemented");
}

void ProtocolManager::onPrimaryConnected()
{
	emit primaryConnected();
	connectAttempts = 0;
}

void ProtocolManager::onPrimaryDisconnected()
{
	if (pPrimary != sender())
		return;

	emit primaryDisconnected();
	spawnReconnect();
}
