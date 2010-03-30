
#include "TorControlManager.h"
#include "TorControlSocket.h"
#include "ProtocolInfoCommand.h"
#include "AuthenticateCommand.h"
#include "SetConfCommand.h"
#include "utils/StringUtil.h"
#include <QHostAddress>
#include <QDir>
#include <QDebug>

using namespace Tor;

TorControlManager::TorControlManager(QObject *parent) :
    QObject(parent), pStatus(NotConnected)
{
	socket = new TorControlSocket;
	QObject::connect(socket, SIGNAL(commandFinished(TorControlCommand*)), this,
					 SLOT(commandFinished(TorControlCommand*)));
	QObject::connect(socket, SIGNAL(connected()), this, SLOT(socketConnected()));
}

void TorControlManager::setStatus(Status n)
{
	if (n == pStatus)
		return;

	Status old = pStatus;
	pStatus = n;

	emit statusChanged(pStatus, old);

	if (pStatus == Connected && old < Connected)
		emit connected();
	else if (pStatus < Connected && old >= Connected)
		emit disconnected();
}

void TorControlManager::setAuthPassword(const QByteArray &password)
{
	pAuthPassword = password;
}

void TorControlManager::connect(const QHostAddress &address, quint16 port)
{
	socket->abort();
	socket->connectToHost(address, port);

	setStatus(Connecting);
}

void TorControlManager::commandFinished(TorControlCommand *command)
{
	if (command->keyword == QLatin1String("PROTOCOLINFO"))
	{
		Q_ASSERT(status() == Authenticating);

		qDebug() << "torctrl: Tor version is" << torVersion();

		if (status() == Authenticating)
			authenticate();
	}
	else if (command->keyword == QLatin1String("AUTHENTICATE"))
	{
		Q_ASSERT(status() == Authenticating);

		if (command->statusCode() != 250)
		{
			qWarning() << "torctrl: Authentication failed with code" << command->statusCode();
			return;
		}

		qDebug() << "torctrl: Authentication successful";
		setStatus(Connected);
	}
}

void TorControlManager::socketConnected()
{
	Q_ASSERT(status() == Connecting);

	qDebug() << "torctrl: Connected socket; querying information";
	setStatus(Authenticating);

	ProtocolInfoCommand *command = new ProtocolInfoCommand(this);
	socket->sendCommand(command, command->build());
}

void TorControlManager::authenticate()
{
	Q_ASSERT(status() == Authenticating);

	AuthenticateCommand *command = new AuthenticateCommand;
	QByteArray data;

	if (pAuthMethods.testFlag(AuthNull))
	{
		qDebug() << "torctrl: Using null authentication";
		data = command->build();
	}
	else if (pAuthMethods.testFlag(AuthHashedPassword))
	{
		qDebug() << "torctrl: Using hashed password authentication";

		if (pAuthPassword.isEmpty())
		{
			qWarning() << "torctrl: Password authentication required and no password is configured";
			delete command;
			return;
		}

		data = command->build(pAuthPassword);
	}
	else
	{
		qWarning("torctrl: No supported authentication methods");
		delete command;
		return;
	}

	socket->sendCommand(command, data);
}

HiddenService *TorControlManager::createHiddenService(const QString &path, const QHostAddress &address, quint16 port)
{
	QDir dir(path);
	QString target = QString("9800 %1:%2").arg(address.toString()).arg(port);

	QList<QPair<QByteArray,QByteArray> > settings;
	settings.append(qMakePair(QByteArray("HiddenServiceDir"), dir.absolutePath().toLocal8Bit()));
	settings.append(qMakePair(QByteArray("HiddenServicePort"), target.toLatin1()));
#if 0
	settings.append(qMakePair(QByteArray("HiddenServiceAuthorizeClient"), QByteArray("stealth bob")));
#endif

	SetConfCommand *command = new SetConfCommand;
	socket->sendCommand(command, command->build(settings));

	return 0;
}
