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

#ifndef RICOTRAY_HPP
#define RICOTRAY_HPP
#include "MainWindow.h"
#include <QSystemTrayIcon>
#include "core/ConversationModel.h"

class RicoTray : public QSystemTrayIcon
{
    Q_OBJECT
    Q_PROPERTY(bool hidden READ isHidden WRITE setHidden NOTIFY hiddenChanged)
    Q_PROPERTY(bool unread READ isUnread WRITE setUnread NOTIFY unreadChanged)
public:
    RicoTray();

signals:
    void iconTriggered();
    void quitTriggered();
    void preferencesTriggered();
    void addContactTriggered();
    void hiddenChanged(bool);
    void unreadChanged(bool);

public slots:
    bool isHidden() const;
    void setHidden(bool);

    bool isUnread() const;
    void setUnread(bool);

    void quit();
    void preferences();
    void addContact();

    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    QIcon std_icon;
    QIcon unread_icon;
    bool unread;
};

#endif // RICOTRAY_HPP
