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

#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>
#include <QList>
#include <QQueue>
#include <QHash>
#include <QAbstractSocket>
#include "ProtocolSocket.h"

class ContactUser;

class ProtocolManager : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(ProtocolManager)

	friend class IncomingSocket;

	Q_PROPERTY(QString host READ host WRITE setHost STORED true)
	Q_PROPERTY(quint16 port READ port WRITE setPort STORED true)

public:
	ContactUser * const user;

	explicit ProtocolManager(ContactUser *user, const QString &host, quint16 port);

	QString host() const { return pHost; }
	void setHost(const QString &host);
	quint16 port() const { return pPort; }
	void setPort(quint16 port);
	QByteArray secret() const { return pSecret; }
	void setSecret(const QByteArray &secret);

	bool isPrimaryConnected() const;
	bool isAnyConnected() const;

	ProtocolSocket *primary() { return pPrimary; }

public slots:
	void connectPrimary();

signals:
	void primaryConnected();
	void primaryDisconnected();

private slots:
	void onPrimaryDisconnected();

private:
	ProtocolSocket *pPrimary, *remotePrimary;

	QString pHost;
	QByteArray pSecret;
	quint16 pPort;

	virtual void addSocket(QTcpSocket *socket, quint8 purpose);
};

/* Do not change this, as it breaks backwards compatibility. Hopefully, it will never be necessary. */
static const quint8 protocolVersion = 0x00;

#endif // PROTOCOLMANAGER_H
