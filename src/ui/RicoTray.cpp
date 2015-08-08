#include "RicoTray.h"
#include "MainWindow.h"
#include <QMessageBox>
#include <QDebug>

RicoTray::RicoTray() :
    QSystemTrayIcon(QIcon(QLatin1String(":/icons/ricochet.svg")), 0)
{
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    show();
}

bool RicoTray::isHidden() const
{
    return hidden;
}

void RicoTray::setHidden(bool p_hidden)
{
    if (hidden != p_hidden)
    {
        hidden = p_hidden;
        setVisible(!hidden);
        emit hiddenChanged(hidden);
    }
}

void RicoTray::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
        emit iconTriggered();
}
