#include "TorControlManager.h"
#include "TorControlSocket.h"
#include "ProtocolInfoCommand.h"
#include <QHostAddress>
#include <QDebug>

using namespace Tor;

TorControlManager::TorControlManager(QObject *parent) :
    QObject(parent)
{
	socket = new TorControlSocket;
	QObject::connect(socket, SIGNAL(commandFinished(TorControlCommand*)), this,
					 SLOT(commandFinished(TorControlCommand*)));
	QObject::connect(socket, SIGNAL(connected()), this, SLOT(authenticate()));
}

void TorControlManager::connect()
{
	socket->connectToHost(QHostAddress(QString("127.0.0.1")), 9051);
}

void TorControlManager::authenticate()
{
	qDebug() << "torctrl: Connected";

	ProtocolInfoCommand *command = new ProtocolInfoCommand;
	socket->sendCommand(command, command->build());
}

void TorControlManager::commandFinished(TorControlCommand *command)
{
	if (dynamic_cast<ProtocolInfoCommand*>(command))
	{
		ProtocolInfoCommand *c = static_cast<ProtocolInfoCommand*>(command);
		qDebug() << "torctrl: Tor version is" << c->torVersion;
	}
}
