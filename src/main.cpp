#include "ui/MainWindow.h"
#include "core/ContactsManager.h"
#include "tor/TorControlManager.h"
#include "protocol/IncomingSocket.h"
#include <QApplication>
#include <QSettings>
#include <QTime>
#include <QDir>
#include <QTranslator>

QSettings *config = 0;

static IncomingSocket *incomingSocket = 0;

static void initSettings();
static void initIncomingSocket();
static bool connectTorControl();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	QTranslator translator;
	translator.load(QString("torim_") + QLocale::system().name(), a.applicationDirPath());
	a.installTranslator(&translator);

	QDir::setCurrent(a.applicationDirPath());

	initSettings();

	/* Seed RNG */
	{
		QTime now = QTime::currentTime();
		qsrand(unsigned(now.second()) * now.msec() * unsigned(a.applicationPid()));
	}

	/* Contacts */
	contactsManager = new ContactsManager;

	/* Incoming socket */
	initIncomingSocket();

	/* Tor control manager */
	bool configured = connectTorControl();
	if (!configured)
		qFatal("Tor control settings aren't configured");

	QObject::connect(torManager, SIGNAL(socksReady()), contactsManager, SLOT(connectToAll()));

	/* Window */
    MainWindow w;
    w.show();

    return a.exec();
}

static void initSettings()
{
	/* Defaults */
	qApp->setOrganizationName(QString("TorIM"));
	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, qApp->applicationDirPath());

	/* Commandline */
	QStringList args = qApp->arguments();
	if (args.size() > 1)
		config = new QSettings(args[1], QSettings::IniFormat);
	else
		config = new QSettings;
}

static void initIncomingSocket()
{
	QHostAddress address(config->value("core/listenIp", QString("127.0.0.1")).toString());
	quint16 port = (quint16)config->value("core/listenPort", 0).toUInt();

	incomingSocket = new IncomingSocket;
	if (!incomingSocket->listen(address, port))
		qFatal("Failed to open incoming socket: %s", qPrintable(incomingSocket->errorString()));
}

static bool connectTorControl()
{
	torManager = new Tor::TorControlManager;

	/* Authentication */
	torManager->setAuthPassword(config->value("tor/authPassword").toByteArray());

	/* Hidden service */
	QString serviceDir = config->value("core/serviceDirectory", QString("data")).toString();

	Tor::HiddenService *service = new Tor::HiddenService(serviceDir);
	service->addTarget(13535, incomingSocket->serverAddress(), incomingSocket->serverPort());

	torManager->addHiddenService(service);

	/* Connect */
	QHostAddress address(config->value("tor/controlIp").value<QString>());
	quint16 port = (quint16)config->value("tor/controlPort", 0).toUInt();

	if (address.isNull() || !port)
		return false;

	torManager->connect(address, port);
	return true;
}
