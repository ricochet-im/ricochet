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

private:
	ProtocolSocket *pPrimary;

	QString pHost;
	QByteArray pSecret;
	quint16 pPort;

	virtual void addSocket(QTcpSocket *socket, quint8 purpose);
};

/* Do not change this, as it breaks backwards compatibility. Hopefully, it will never be necessary. */
static const quint8 protocolVersion = 0x00;

#endif // PROTOCOLMANAGER_H
