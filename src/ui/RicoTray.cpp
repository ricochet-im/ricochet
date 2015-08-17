#include "RicoTray.h"
#include "MainWindow.h"
#include <QMessageBox>
#include <QMenu>
#include <QDebug>

RicoTray::RicoTray() :
    std_icon{QIcon{QLatin1String{":/icons/ricochet.svg"}}},
    unread_icon{QIcon{QLatin1String{":/icons/ricochet_dot.svg"}}},
    unread{false}
{
    setIcon(std_icon);
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

bool RicoTray::isUnread() const
{
    return unread;
}

void RicoTray::setUnread(bool p_unread)
{
    unread = p_unread;
    qDebug() << "Setting unread: " << p_unread;
    if (unread) setIcon(unread_icon);
    else setIcon(std_icon);

    emit unreadChanged(unread);
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
