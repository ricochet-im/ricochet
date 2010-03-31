#include "TorControlManager.h"
#include "TorControlSocket.h"
#include "ProtocolInfoCommand.h"
#include "AuthenticateCommand.h"
#include "SetConfCommand.h"
#include "GetConfCommand.h"
#include "utils/StringUtil.h"
#include <QHostAddress>
#include <QDir>
#include <QDebug>

Tor::TorControlManager *torManager = 0;

using namespace Tor;

TorControlManager::TorControlManager(QObject *parent)
	: QObject(parent), pStatus(NotConnected)
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

		getSocksInfo();
		publishServices();
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

void TorControlManager::getSocksInfo()
{
	Q_ASSERT(isConnected());

	qDebug() << "torctrl: Querying for SOCKS connection settings";

	GetConfCommand *command = new GetConfCommand;

	QList<QByteArray> options;
	options << QByteArray("SocksPort") << QByteArray("SocksListenAddress");

	socket->sendCommand(command, command->build(options));
}

void TorControlManager::addHiddenService(HiddenService *service)
{
	if (pServices.contains(service))
		return;

	pServices.append(service);
}

void TorControlManager::publishServices()
{
	Q_ASSERT(isConnected());
	if (pServices.isEmpty())
		return;

	for (QList<HiddenService*>::Iterator it = pServices.begin(); it != pServices.end(); ++it)
	{
		HiddenService *service = *it;
		QDir dir(service->dataPath);

		qDebug() << "torctrl: Configuring hidden service at" << service->dataPath;

		QList<QPair<QByteArray,QByteArray> > settings;
		settings.append(qMakePair(QByteArray("HiddenServiceDir"), dir.absolutePath().toLocal8Bit()));

		const QList<HiddenService::Target> &targets = service->targets();
		for (QList<HiddenService::Target>::ConstIterator tit = targets.begin(); tit != targets.end(); ++tit)
		{
			QString target = QString("%1 %2:%3").arg(tit->servicePort).arg(tit->targetAddress.toString())
							 .arg(tit->targetPort);
			settings.append(qMakePair(QByteArray("HiddenServicePort"), target.toLatin1()));
		}

		//settings.append(qMakePair(QByteArray("HiddenServiceAuthorizeClient"), QByteArray("stealth bob")));

		SetConfCommand *command = new SetConfCommand;
		socket->sendCommand(command, command->build(settings));
	}
}

HiddenService::HiddenService(const QString &p)
	: dataPath(p), pStatus(Offline)
{
}

void HiddenService::addTarget(const Target &target)
{
	pTargets.append(target);
}

void HiddenService::addTarget(quint16 servicePort, QHostAddress targetAddress, quint16 targetPort)
{
	Target t = { targetAddress, servicePort, targetPort };
	pTargets.append(t);
}
