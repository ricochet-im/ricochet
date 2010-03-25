#include "TorControlManager.h"
#include "TorControlSocket.h"
#include "ProtocolInfoCommand.h"
#include "AuthenticateCommand.h"
#include <QHostAddress>
#include <QDebug>

using namespace Tor;

TorControlManager::TorControlManager(QObject *parent) :
    QObject(parent)
{
	socket = new TorControlSocket;
	QObject::connect(socket, SIGNAL(commandFinished(TorControlCommand*)), this,
					 SLOT(commandFinished(TorControlCommand*)));
	QObject::connect(socket, SIGNAL(connected()), this, SLOT(queryInfo()));
}

void TorControlManager::connect()
{
	socket->connectToHost(QHostAddress(QString("127.0.0.1")), 9051);
}

void TorControlManager::commandFinished(TorControlCommand *command)
{
	if (command->keyword == QLatin1String("PROTOCOLINFO"))
	{
		qDebug() << "torctrl: Tor version is" << torVersion();

		authenticate();
	}
	else if (command->keyword == QLatin1String("AUTHENTICATE"))
	{
		if (command->statusCode() != 250)
			qWarning() << "torctrl: Authentication failed with code" << command->statusCode();
		else
			qDebug() << "torctrl: Authentication successful";
	}
}

void TorControlManager::queryInfo()
{
	qDebug() << "torctrl: Connected";

	ProtocolInfoCommand *command = new ProtocolInfoCommand(this);
	socket->sendCommand(command, command->build());
}

void TorControlManager::authenticate()
{
	AuthenticateCommand *command = new AuthenticateCommand;
	QByteArray data;

	if (pAuthMethods.testFlag(AuthNull))
	{
		qDebug() << "torctrl: Using 'NULL' authentication";
		data = command->build();
	}
	else if (pAuthMethods.testFlag(AuthHashedPassword))
	{
		qDebug() << "torctrl: Using hashed password authentication";
		data = command->build(QByteArray("test"));
	}
	else
	{
		qWarning("torctrl: No supported authentication methods");
		delete command;
		return;
	}

	socket->sendCommand(command, data);
}
