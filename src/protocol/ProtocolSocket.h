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

#ifndef PROTOCOLSOCKET_H
#define PROTOCOLSOCKET_H

#include <QObject>
#include <QTcpSocket>
#include <QQueue>
#include <QHash>

class ProtocolManager;
class ProtocolCommand;

class ProtocolSocket : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(ProtocolSocket)

public:
	ProtocolManager * const manager;
	QTcpSocket * const socket;

	/* Create with an established and authenticated socket (incoming connections) */
	explicit ProtocolSocket(QTcpSocket *socket, ProtocolManager *manager);
	/* Create with a new socket */
	explicit ProtocolSocket(ProtocolManager *manager);

	/* Returns true if the socket is connected and ready (i.e. authenticated) */
	bool isConnected() const;
	bool isConnecting() const;

	void connectToHost(const QString &host, quint16 port);

	/* Get an available identifier; not reserved, must be followed by sendCommand immediately. */
	quint16 getIdentifier() const;

	void sendCommand(ProtocolCommand *command);

signals:
	/* Connected and authenticated */
	void socketReady();
	/* Disconnected from an authenticated connection */
	void disconnected();
	/* Connection attempt failed or disconnected from an unauthenticated connection */
	void connectFailed();

public slots:
	void abort();
	void abortConnectionAttempt();

private slots:
	void sendAuth();
	void flushCommands();

	void read();
	void socketDisconnected();

private:
	QQueue<ProtocolCommand*> commandQueue;
	QHash<quint16,ProtocolCommand*> pendingCommands;

	bool active, authPending, authFinished;

	void setupSocket();
};

#endif // PROTOCOLSOCKET_H
