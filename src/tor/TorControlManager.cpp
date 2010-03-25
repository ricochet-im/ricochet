#include "TorControlManager.h"
#include "TorControlSocket.h"
#include <QHostAddress>

using namespace Tor;

TorControlManager::TorControlManager(QObject *parent) :
    QObject(parent)
{
	socket = new TorControlSocket;
	QObject::connect(socket, SIGNAL(connected()), this, SLOT(authenticate()));
}

void TorControlManager::connect()
{
	socket->connectToHost(QHostAddress(QString("127.0.0.1")), 9051);
}

void TorControlManager::authenticate()
{
	qDebug("Connected");
	socket->write("PROTOCOLINFO 1\r\n");
}
