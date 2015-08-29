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

#include "TrayIcon.h"
#include <QQuickWindow>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QPainter>
#include <QPixmap>

TrayIcon::TrayIcon(QObject *parent)
    : QObject(parent)
    , enabled(false)
    , showMessagesEnabled(false)
    , notification(Default)
    , tray(nullptr)
    , window(nullptr)
    , contextMenu(nullptr)
    , showAction(nullptr)
    , hideAction(nullptr)
{
    if (isAvailable()) {
        contextMenu = new QMenu();
        showAction = new QAction(tr("Show"), this);
        hideAction = new QAction(tr("Hide"), this);
        quitAction = new QAction(tr("Quit"), this);

        connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

        contextMenu->addAction(quitAction);

        setupIcons();
        tray = new QSystemTrayIcon(this);
        tray->setIcon(getNotificationIcon(notification));
        tray->setContextMenu(contextMenu);
        if (isEnabled()) tray->show();

        connect(tray, &QSystemTrayIcon::activated, this, &TrayIcon::handleClick);
    }
}

void TrayIcon::setWindow(QWindow *w)
{
    Q_ASSERT(window == nullptr); // only set window once
    window = w;
    connect(showAction, &QAction::triggered, window, &QWindow::show);
    connect(hideAction, &QAction::triggered, window, &QWindow::hide);
}

void TrayIcon::setEnabled(bool e)
{
    if (e == enabled) return;
    enabled = e;

    if (enabled) {
        tray->show();
    } else {
        tray->hide();
    }

    emit enabledChanged();
}

bool TrayIcon::isEnabled() const
{
    return enabled;
}

void TrayIcon::setMessagesEnabled(bool b)
{
    if (b == showMessagesEnabled) return;

    showMessagesEnabled = b;
    emit messagesEnabledChanged();
}

bool TrayIcon::getMessagesEnabled() const
{
    return showMessagesEnabled;
}

void TrayIcon::setNotification(NotificatonState n)
{
    if (!isEnabled()) return;
    if (notification == n) return;

    notification = n;
    tray->setIcon(getNotificationIcon(notification));

    emit notificationStateChanged();
}

TrayIcon::NotificatonState TrayIcon::getCurrentNotification() const
{
    return notification;
}

void TrayIcon::showText(const QString &title, const QString &text)
{
    if (isEnabled()) {
        tray->showMessage(title, text);
    }
}

void TrayIcon::notificationAcknowledged()
{
    setNotification(Default);
}

void TrayIcon::notifyMessage()
{
    setNotification(Message);
}

bool TrayIcon::isAvailable()
{
    return QSystemTrayIcon::isSystemTrayAvailable();
}

void TrayIcon::handleClick(QSystemTrayIcon::ActivationReason reason)
{
    Q_ASSERT(window != nullptr);
    switch(reason) {
        case QSystemTrayIcon::Trigger:
            if (getCurrentNotification() != Default) {
                notificationAcknowledged();
            }
            if (!window->isVisible()) {
                window->show();
            } else {
                window->hide();
            }
            break;
        case QSystemTrayIcon::Context:
            if (window->isVisible()) {
                contextMenu->removeAction(showAction);
                contextMenu->insertAction(quitAction, hideAction);
            } else {
                contextMenu->removeAction(hideAction);
                contextMenu->insertAction(quitAction, showAction);
            }
            break;
        default:
            break;
    }
}

void TrayIcon::setupIcons()
{
    normalIcon = QIcon(QStringLiteral(":/icons/ricochet.svg"));
    messageIcon = QIcon(QStringLiteral(":/icons/message.svg"));
}

const QIcon &TrayIcon::getNotificationIcon(TrayIcon::NotificatonState n) const
{
    switch(n) {
        case Message:
            return messageIcon;
        case Default:
        default:
            return normalIcon;
    }
}
