#include <QtGui/QApplication>
#include "ui/MainWindow.h"
#include "tor/TorControlManager.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.show();

	TorControlManager *torManager = new TorControlManager;
	torManager->connect();

    return a.exec();
}
