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
	void socketReady();

public slots:
	void abort();

private slots:
	void sendAuth();
	void flushCommands();

	void read();

private:
	QQueue<ProtocolCommand*> commandQueue;
	QHash<quint16,ProtocolCommand*> pendingCommands;

	bool authPending, authFinished;

	void setupSocket();
};

#endif // PROTOCOLSOCKET_H
