/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

#ifndef HOMESCREEN_H
#define HOMESCREEN_H

#include <QWidget>
#include <QPointer>

class NotificationWidget;
class QLabel;
class QAction;

class HomeScreen : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(HomeScreen)

public:
    explicit HomeScreen(QWidget *parent = 0);

private slots:
    void updateTorStatus();
    void enableTorNotification();

    void startTorConfig();

    void startAddContact();

private:
    QLabel *avatar;

    QList<QAction*> buttonActions;
    QAction *actAddContact, *actChangeAvatar, *actOpenDownloads, *actTestConnection, *actOptions;
    QAction *actTorConfig;

    QLabel *torStatus, *torInfo;
    QPointer<NotificationWidget> torNotification;
    bool torNotificationEnabled;

    void createAvatar();
    void createActions();
    QLayout *createButtons();
    QWidget *createStatus();
};

#endif // HOMESCREEN_H
