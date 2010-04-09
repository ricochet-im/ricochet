#include "TorServiceTest.h"
#include "TorControlManager.h"

using namespace Tor;

TorServiceTest::TorServiceTest(TorControlManager *m)
	: QObject(m), manager(m), socket(new QTcpSocket(this)), state(-1)
{
}

void TorServiceTest::connectToHost(const QString &host, quint16 port)
{
	if (socket->state() != QAbstractSocket::UnconnectedState)
		socket->abort();

	state = -1;
	socket->connectToHost(host, port);
}

void TorServiceTest::socketConnected()
{
	state = 1;
	emit finished(true);
	emit success();
}

void TorServiceTest::socketError(QAbstractSocket::SocketError)
{
	state = 0;
	emit finished(false);
	emit failure();
}
