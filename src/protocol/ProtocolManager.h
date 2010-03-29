#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>
#include <QList>
#include <QQueue>
#include <QHash>
#include <QAbstractSocket>

class QTcpSocket;
class ProtocolCommand;

typedef unsigned char uchar;

class ProtocolManager : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(ProtocolManager)

	friend class IncomingSocket;

	Q_PROPERTY(QString host READ host WRITE setHost STORED true)
	Q_PROPERTY(quint16 port READ port WRITE setPort STORED true)

public:
	explicit ProtocolManager(const QString &host, quint16 port, QObject *parent = 0);

	QString host() const { return pHost; }
	void setHost(const QString &host);
	quint16 port() const { return pPort; }
	void setPort(quint16 port);
	QByteArray secret() const { return pSecret; }
	void setSecret(const QByteArray &secret);

	bool isPrimaryConnected() const;
	bool isAnyConnected() const;

	/* Get an available identifier; not reserved, must be followed by sendCommand immediately. */
	quint16 getIdentifier() const;
	void sendCommand(ProtocolCommand *command, bool ordered = true);

public slots:
	void connectPrimary();
	void connectAnother();

signals:
	void primaryConnected();
	void primaryDisconnected();

private slots:
	void socketConnected(QTcpSocket *socket = 0);
	void socketDisconnected();
	void socketError(QAbstractSocket::SocketError error);
	void socketReadable();

private:
	/* The socket pool holds open (or connecting) sockets, which may or may not be busy. The
	 * primary socket is used for ordered commands, and is not contained in the socketPool.
	 * Socket pool sockets are intended for use by unordered commands. */
	QList<QTcpSocket*> socketPool;
	QTcpSocket *primarySocket;

	/* Queue of ordered commands waiting to be sent */
	QQueue<ProtocolCommand*> commandQueue;
	/* Queue of unordered commands waiting to be sent */
	QQueue<ProtocolCommand*> unorderedCommandQueue;

	/* Map of identifiers to commands with pending replies; shared between all sockets. */
	QHash<quint16,ProtocolCommand*> pendingCommands;

	QString pHost;
	QByteArray pSecret;
	quint16 pPort;

	virtual void addSocket(QTcpSocket *socket, quint8 purpose);
	void socketAuthenticated(QTcpSocket *socket);
};

/* Do not change this, as it breaks backwards compatibility. Hopefully, it will never be necessary. */
static const quint8 protocolVersion = 0x00;

#endif // PROTOCOLMANAGER_H
