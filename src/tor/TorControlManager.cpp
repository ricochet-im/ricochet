#include "main.h"
#include "TorControlManager.h"
#include "TorControlSocket.h"
#include "HiddenService.h"
#include "ProtocolInfoCommand.h"
#include "AuthenticateCommand.h"
#include "SetConfCommand.h"
#include "GetConfCommand.h"
#include "utils/StringUtil.h"
#include <QHostAddress>
#include <QDir>
#include <QNetworkProxy>
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
	QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
}

QNetworkProxy TorControlManager::connectionProxy()
{
	return QNetworkProxy(QNetworkProxy::Socks5Proxy, pSocksAddress.toString(), pSocksPort);
}

void TorControlManager::setStatus(Status n)
{
	if (n == pStatus)
		return;

	Status old = pStatus;
	pStatus = n;

	if (old == Error)
		pErrorMessage.clear();

	emit statusChanged(pStatus, old);

	if (pStatus == Connected && old < Connected)
		emit connected();
	else if (pStatus < Connected && old >= Connected)
		emit disconnected();
}

void TorControlManager::setError(const QString &message)
{
	pErrorMessage = message;
	setStatus(Error);

	qWarning() << "torctrl: Error:" << pErrorMessage;

	socket->abort();
}

QString TorControlManager::statusText() const
{
	switch (pStatus)
	{
	case Error:
		return pErrorMessage.isEmpty() ? tr("An unknown error occurred") : pErrorMessage;
	case NotConnected:
		return tr("Not connected to Tor");
	case Connecting:
	case Authenticating:
		return tr("Connecting to Tor");
	case Connected:
		return tr("Connected to Tor");
	default:
		return QString();
	}
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

		if (command->statusCode() == 515)
		{
			setError(tr("Authentication failed - incorrect password"));
		}
		else if (command->statusCode() != 250)
		{
			setError(tr("Authentication failed (error %1)").arg(command->statusCode()));
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

void TorControlManager::socketDisconnected()
{
	/* Clear some internal state */
	pTorVersion.clear();
	pSocksAddress.clear();
	pSocksPort = 0;
	pAuthMethods = AuthUnknown;

	/* This emits the disconnected() signal as well */
	setStatus(NotConnected);
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
			setError(tr("Tor requires a control password to connect, but no password is configured."));
			delete command;
			return;
		}

		data = command->build(pAuthPassword);
	}
	else
	{
		setError(tr("Tor is not configured to accept any supported authentication methods."));
		delete command;
		return;
	}

	socket->sendCommand(command, data);
}

void TorControlManager::getSocksInfo()
{
	Q_ASSERT(isConnected());

	/* If these are set in the config, they override the automatic behavior. */
	QHostAddress forceAddress(config->value("tor/socksIp").toString());
	quint16 port = (quint16)config->value("tor/socksPort").toUInt();

	if (!forceAddress.isNull() && port)
	{
		qDebug() << "torctrl: Using manually specified SOCKS connection settings";
		pSocksAddress = forceAddress;
		pSocksPort = port;
		emit socksReady();
		return;
	}

	qDebug() << "torctrl: Querying for SOCKS connection settings";

	GetConfCommand *command = new GetConfCommand;
	QObject::connect(command, SIGNAL(replyFinished()), this, SLOT(getSocksInfoReply()));

	QList<QByteArray> options;
	options << QByteArray("SocksPort") << QByteArray("SocksListenAddress");

	socket->sendCommand(command, command->build(options));
}

void TorControlManager::getSocksInfoReply()
{
	GetConfCommand *command = qobject_cast<GetConfCommand*>(sender());
	if (!command || !isConnected())
		return;

	/* If there is a SocksListenAddress line that is either null or has an IP with no port,
	 * Tor is listening on that IP (default 127.0.0.1) and the value from SocksPort.
	 *
	 * If neither of those cases is true, SocksPort should be ignored, and Tor is listening
	 * only on the IP:port pairs from SocksListenAddress.
	 */

	QList<QByteArray> listenAddresses = command->getList(QByteArray("SocksListenAddress"));

	quint16 defaultPort = 0;
	QByteArray socksPortData;
	if (command->get(QByteArray("SocksPort"), socksPortData))
		defaultPort = (quint16)socksPortData.toUInt();
	if (!defaultPort)
		defaultPort = 9050;

	for (QList<QByteArray>::Iterator it = listenAddresses.begin(); it != listenAddresses.end(); ++it)
	{
		QHostAddress address;
		quint16 port = 0;

		if (!it->isNull())
		{
			int sepp = it->indexOf(':');
			address.setAddress(QString::fromLatin1(it->mid(0, sepp)));
			if (sepp >= 0)
				port = (quint16)it->mid(sepp+1).toUInt();
		}

		if (address.isNull())
			address.setAddress(QString("127.0.0.1"));
		if (!port)
			port = defaultPort;

		/* Use the first address that matches the one used for this control connection. If none do,
		 * just use the first address and rely on the user to reconfigure if necessary (not a problem;
		 * their setup is already very customized) */
		if (pSocksAddress.isNull() || address == socket->peerAddress())
		{
			pSocksAddress = address;
			pSocksPort = port;

			/* No need to parse the others if we got what we wanted */
			if (address == socket->peerAddress())
				break;
		}
	}

	if (pSocksAddress.isNull())
	{
		pSocksAddress.setAddress(QString("127.0.0.1"));
		pSocksPort = defaultPort;
	}

	qDebug().nospace() << "torctrl: SOCKS address is " << pSocksAddress.toString() << ":" << pSocksPort;
	emit socksReady();
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

	if (config->value(QString("core/neverPublishService"), false).toBool())
	{
		qDebug() << "torctrl: Skipping service publication because neverPublishService is enabled";

		/* Call servicePublished under the assumption that they're published externally. */
		for (QList<HiddenService*>::Iterator it = pServices.begin(); it != pServices.end(); ++it)
			(*it)->servicePublished();

		return;
	}

	SetConfCommand *command = new SetConfCommand;
	QList<QPair<QByteArray,QByteArray> > settings;

	for (QList<HiddenService*>::Iterator it = pServices.begin(); it != pServices.end(); ++it)
	{
		HiddenService *service = *it;
		QDir dir(service->dataPath);

		qDebug() << "torctrl: Configuring hidden service at" << service->dataPath;

		settings.append(qMakePair(QByteArray("HiddenServiceDir"), dir.absolutePath().toLocal8Bit()));

		const QList<HiddenService::Target> &targets = service->targets();
		for (QList<HiddenService::Target>::ConstIterator tit = targets.begin(); tit != targets.end(); ++tit)
		{
			QString target = QString("%1 %2:%3").arg(tit->servicePort).arg(tit->targetAddress.toString())
							 .arg(tit->targetPort);
			settings.append(qMakePair(QByteArray("HiddenServicePort"), target.toLatin1()));
		}

		//settings.append(qMakePair(QByteArray("HiddenServiceAuthorizeClient"), QByteArray("stealth bob")));

		QObject::connect(command, SIGNAL(setConfSucceeded()), service, SLOT(servicePublished()));
	}

	socket->sendCommand(command, command->build(settings));
}
