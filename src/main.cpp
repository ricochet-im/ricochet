#include <QApplication>
#include <QSettings>
#include "ui/MainWindow.h"
#include "tor/TorControlManager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	a.setOrganizationName(QString("TorIM"));

	QSettings::setDefaultFormat(QSettings::IniFormat);
	QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, a.applicationDirPath());

    MainWindow w;
    w.show();

	Tor::TorControlManager *torManager = new Tor::TorControlManager;
	torManager->connect();

    return a.exec();
}
