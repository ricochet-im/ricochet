#include "RicoTray.h"
#include "MainWindow.h"

RicoTray::RicoTray(MainWindow *p_main) :
    QSystemTrayIcon(QIcon(QLatin1String(":/icons/ricochet.svg")), p_main),
    main(p_main)
{
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

void RicoTray::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
    {
        if (main == nullptr) {
            main = new MainWindow();
        } else {
            main = nullptr;
        }
    }
}

