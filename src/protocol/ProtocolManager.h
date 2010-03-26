#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>
#include <QList>
#include <QQueue>
#include <QHash>

class QTcpSocket;
class ProtocolCommand;

class ProtocolManager : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(ProtocolManager)

	Q_PROPERTY(QString host READ host WRITE setHost STORED true)
	Q_PROPERTY(quint16 port READ port WRITE setPort STORED true)

public:
    explicit ProtocolManager(const QString &host, quint16 port, QObject *parent = 0);

	QString host() const { return pHost; }
	void setHost(const QString &host);
	quint16 port() const { return pPort; }
	void setPort(quint16 port);

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

private slots:
	void socketConnected();
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
	quint16 pPort;
};

#endif // PROTOCOLMANAGER_H
