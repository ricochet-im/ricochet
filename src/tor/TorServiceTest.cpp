#include "TorServiceTest.h"
#include "TorControlManager.h"
#include <QNetworkProxy>

using namespace Tor;

TorServiceTest::TorServiceTest(TorControlManager *m)
	: QObject(m), manager(m), socket(new QTcpSocket(this)), state(-1)
{
	connect(manager, SIGNAL(socksReady()), this, SLOT(socksReady()));

	connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
			SLOT(socketError(QAbstractSocket::SocketError)));
}

void TorServiceTest::connectToHost(const QString &host, quint16 port)
{
	if (socket->state() != QAbstractSocket::UnconnectedState)
		socket->abort();

	this->host = host;
	this->port = port;
	state = -1;

	if (!manager->isSocksReady())
	{
		qDebug() << "Tor self-test waiting for SOCKS to be ready";
		return;
	}

	socket->setProxy(manager->connectionProxy());
	socket->connectToHost(host, port);
}

void TorServiceTest::socketConnected()
{
	state = 1;
	emit finished(true);
	emit success();

	socket->close();
}

void TorServiceTest::socketError(QAbstractSocket::SocketError)
{
	state = 0;
	emit finished(false);
	emit failure();
}

void TorServiceTest::socksReady()
{
	if (state >= 0 || host.isEmpty())
		return;

	connectToHost(host, port);
}
