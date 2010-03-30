#ifndef INCOMINGSOCKET_H
#define INCOMINGSOCKET_H

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QBasicTimer>

class QTcpServer;
class QTcpSocket;

class IncomingSocket : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(IncomingSocket)

public:
	explicit IncomingSocket(QObject *parent = 0);

	bool listen(const QHostAddress &address, quint16 port);
	QString errorString() const;

	QHostAddress serverAddress() const;
	quint16 serverPort() const;

private slots:
	void incomingConnection();

	void readSocket();
	void removeSocket(QTcpSocket *socket = 0);

protected:
	virtual void timerEvent(QTimerEvent *);

private:
	QTcpServer *server;
	QList<QTcpSocket*> pendingSockets;
	QBasicTimer expireTimer;
};

#endif // INCOMINGSOCKET_H
