#include "ui/MainWindow.h"
#include "core/ContactsManager.h"
#include "tor/TorControlManager.h"
#include "protocol/IncomingSocket.h"
#include <QApplication>
#include <QSettings>
#include <QTime>

#include "protocol/ProtocolManager.h"
#include "protocol/PingCommand.h"

static bool connectTorControl();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setOrganizationName(QString("TorIM"));

	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, a.applicationDirPath());

	{
		QTime now = QTime::currentTime();
		qsrand(unsigned(now.second()) * now.msec() * unsigned(a.applicationPid()));
	}

	/* Initialization */
	contactsManager = new ContactsManager;

	bool configured = connectTorControl();
	if (!configured)
		qFatal("Tor control settings aren't configured");

	IncomingSocket *incoming = new IncomingSocket;
	if (!incoming->listen(QHostAddress(QString("127.0.0.1")), 7777))
		qFatal("Failed to open incoming socket: %s", qPrintable(incoming->errorString()));

	ProtocolManager *testManager = new ProtocolManager(QString("192.168.1.1"), 7777);
	testManager->connectPrimary();

	PingCommand *command = new PingCommand;
	command->send(testManager);

	/* Window */
    MainWindow w;
    w.show();

    return a.exec();
}

static bool connectTorControl()
{
	QSettings settings;
	QHostAddress address(settings.value("tor/controlIp").value<QString>());
	quint16 port = (quint16)settings.value("tor/controlPort", 0).toUInt();

	if (address.isNull() || !port)
		return false;

	Tor::TorControlManager *torManager = new Tor::TorControlManager;
	torManager->setAuthPassword(settings.value("tor/authPassword").toByteArray());
	torManager->connect(address, port);

	return true;
}
