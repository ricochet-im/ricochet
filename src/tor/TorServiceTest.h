#ifndef TORSERVICETEST_H
#define TORSERVICETEST_H

#include <QObject>
#include <QTcpSocket>

namespace Tor
{

class TorControlManager;

class TorServiceTest : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(TorServiceTest)

public:
	TorControlManager * const manager;

	explicit TorServiceTest(TorControlManager *manager);

	void connectToHost(const QString &host, quint16 port);

	bool isSuccessful() const { return state == 1; }
	bool isFinished() const { return state >= 0; }

signals:
	void success();
	void failure();
	void finished(bool success);

private slots:
	void socketConnected();
	void socketError(QAbstractSocket::SocketError error);

	void socksReady();

private:
	QTcpSocket *socket;
	QString host;
	quint16 port;
	int state;
};

}

#endif // TORSERVICETEST_H
