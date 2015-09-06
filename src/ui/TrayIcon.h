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

#ifndef TRAYICON_H
#define TRAYICON_H
#include <QSystemTrayIcon>
#include <QMenu>

class TrayIcon : public QSystemTrayIcon
{
    Q_OBJECT
    Q_PROPERTY(bool unread WRITE setUnread)
public:
    TrayIcon(QIcon std_icon, QIcon unread_icon);
    ~TrayIcon();

    void setStdIcon(QIcon std_icon);
    void setUnreadIcon(QIcon unread_icon);

    // changes icon to m_std_icon
    void resetIcon();

    // changes icon to m_unread_icon
    void setUnread(bool unread);

public slots:
    void activated(QSystemTrayIcon::ActivationReason);
    void openPreferences();
    void addContact();
    void copyMyId();
    void quitApplication();

signals:
    void toggleWindow();
    void preferences();
    void contact();
    void copyId();
    void quit();

private:
    QIcon m_std_icon;
    QIcon m_unread_icon;

    QMenu* m_context_menu;
};

#endif // TRAYICON_H

