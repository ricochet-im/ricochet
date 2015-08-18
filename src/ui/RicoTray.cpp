/* Ricochet - https://ricochet.im/
 * Copyright (C) 2015, Kacper Kołodziej <kacper@kolodziej.in>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
    QAction* addContactAction = menu->addAction(RicoTray::tr("Add Contact"));
    connect(addContactAction, SIGNAL(triggered()), this, SLOT(addContact()));
    // menu -> preferences
    QAction* preferencesAction = menu->addAction(RicoTray::tr("Preferences"));
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(preferences()));

    // menu -> quit
    menu->addSeparator();
    QAction* quitAction = menu->addAction(RicoTray::tr("Quit"));
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
