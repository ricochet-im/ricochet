#include <QApplication>
#include <QSettings>
#include "ui/MainWindow.h"
#include "tor/TorControlManager.h"

static bool connectTorControl();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setOrganizationName(QString("TorIM"));

	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, a.applicationDirPath());

    MainWindow w;
    w.show();

	bool configured = connectTorControl();
	if (!configured)
		qFatal("Tor control settings aren't configured");

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
}
