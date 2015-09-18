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

#include <QApplication>

TrayIcon::TrayIcon(const QIcon& std_icon, const QIcon& unread_icon) :
        m_std_icon(std_icon),
        m_unread_icon(unread_icon)
{
    connect(this, &QSystemTrayIcon::activated, this, &TrayIcon::onActivated);
    setIcon(m_std_icon);

    m_context_menu = new QMenu();
    m_context_menu->addAction(tr("Preferences"), this, SIGNAL(preferences()));
    m_context_menu->addAction(tr("Add Contact"), this, SIGNAL(addContact()));
    m_context_menu->addAction(tr("Copy My ID"), this, SIGNAL(copyId()));
    m_context_menu->addSeparator();
    m_context_menu->addAction(tr("Quit"), qApp, SLOT(quit()));
    setContextMenu(m_context_menu);

    show();
}

TrayIcon::~TrayIcon()
{
    delete m_context_menu;
}

void TrayIcon::resetIcon()
{
    setIcon(m_std_icon);
}

void TrayIcon::setUnread(bool unread)
{
    if (unread)
        setIcon(m_unread_icon);
    else
        resetIcon();
}

void TrayIcon::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger)
        emit toggleWindow();
}
