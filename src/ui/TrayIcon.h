/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
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

#include <QObject>
#include <QSystemTrayIcon>

class QWindow;
class QMenu;
class QAction;

class TrayIcon : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isAvailable READ isAvailable CONSTANT)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool messagesEnabled READ getMessagesEnabled WRITE setMessagesEnabled NOTIFY messagesEnabledChanged)

public:
    enum NotificatonState {
        Default,
        Message
    };

    TrayIcon(QObject *parent = 0);

    void setWindow(QWindow*);

    void setEnabled(bool);
    bool isEnabled() const;

    void setMessagesEnabled(bool);
    bool getMessagesEnabled() const;

    void setNotification(NotificatonState);
    NotificatonState getCurrentNotification() const;

    Q_INVOKABLE void showText(const QString& title, const QString& text);

    Q_INVOKABLE void notificationAcknowledged();
    Q_INVOKABLE void notifyMessage();


    static bool isAvailable();

signals:
    void enabledChanged();
    void notificationStateChanged();
    void messagesEnabledChanged();

private:
    void handleClick(QSystemTrayIcon::ActivationReason);
    void setupIcons();
    const QIcon& getNotificationIcon(NotificatonState) const;

private:
    bool enabled;
    bool showMessagesEnabled;
    NotificatonState notification;

    QSystemTrayIcon* tray;
    QWindow* window;

    QMenu* contextMenu;
    QAction* showAction;
    QAction* hideAction;
    QAction* quitAction;

    QIcon normalIcon;
    QIcon messageIcon;
};

#endif // TRAYICON_H
