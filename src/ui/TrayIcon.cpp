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

#include "TrayIcon.h"

TrayIcon::TrayIcon(QIcon std_icon, QIcon unread_icon) :
        m_std_icon(std_icon),
        m_unread_icon(unread_icon)
{
    connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(activated(QSystemTrayIcon::ActivationReason)));
    setIcon(m_std_icon);

    m_context_menu = new QMenu();
    m_context_menu->addAction(tr("Preferences"), this, SLOT(openPreferences()));
    m_context_menu->addAction(tr("Add contact"), this, SLOT(addContact()));
    m_context_menu->addAction(tr("Copy my ID"), this, SLOT(copyMyId()));
    m_context_menu->addSeparator();
    m_context_menu->addAction(tr("Quit"), this, SLOT(quitApplication()));
    setContextMenu(m_context_menu);

    show();
}

TrayIcon::~TrayIcon()
{
    delete m_context_menu;
}

void TrayIcon::setStdIcon(QIcon std_icon)
{
    m_std_icon = std_icon;
}

void TrayIcon::setUnreadIcon(QIcon unread_icon)
{
    m_unread_icon = unread_icon;
}

void TrayIcon::resetIcon()
{
    setIcon(m_std_icon);
}

void TrayIcon::setUnread(bool unread)
{
    if (unread) setIcon(m_unread_icon);
    else resetIcon();
}

void TrayIcon::activated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
        emit toggleWindow();
}

void TrayIcon::openPreferences()
{
    emit preferences();
}

void TrayIcon::addContact()
{
    emit contact();
}

void TrayIcon::copyMyId()
{
    emit copyId();
}

void TrayIcon::quitApplication()
{
    emit quit();
}
