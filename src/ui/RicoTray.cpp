#include "RicoTray.h"
#include "MainWindow.h"
#include <QMessageBox>
#include <QMenu>
#include <QDebug>

RicoTray::RicoTray() :
    QSystemTrayIcon(QIcon(QLatin1String(":/icons/ricochet.svg")), 0)
{
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    QMenu* menu = new QMenu;
    // menu -> add contact
    QAction* addContactAction = menu->addAction(tr("Add Contact"));
    connect(addContactAction, SIGNAL(triggered()), this, SLOT(addContact()));
    // menu -> preferences
    QAction* preferencesAction = menu->addAction(tr("Preferences"));
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(preferences()));

    // menu -> quit
    menu->addSeparator();
    QAction* quitAction = menu->addAction(tr("Quit"));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(quit()));

    setContextMenu(menu);

    show();
}

bool RicoTray::isHidden() const
{
    return !isVisible();
}

void RicoTray::setHidden(bool p_hidden)
{
    if (isHidden() != p_hidden)
    {
        setVisible(!p_hidden);
        emit hiddenChanged(p_hidden);
    }
}

void RicoTray::quit()
{
    emit quitTriggered();
}

void RicoTray::preferences()
{
    emit preferencesTriggered();
}

void RicoTray::addContact()
{
    emit addContactTriggered();
}

void RicoTray::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        emit iconTriggered();
    }
}
