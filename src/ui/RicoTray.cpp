#include "RicoTray.h"
#include "MainWindow.h"
#include <QMessageBox>
#include <QDebug>

RicoTray::RicoTray() :
    QSystemTrayIcon(QIcon(QLatin1String(":/icons/ricochet.svg")), 0)
{
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
}

bool RicoTray::isEnabled() const
{
    return enabled;
}

void RicoTray::setEnabled(bool p_enabled)
{
    if (enabled != p_enabled)
    {
        enabled = p_enabled;
        setVisible(enabled);
        emit enabledChanged(enabled);
    }
}

void RicoTray::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
        emit iconTriggered();
}
