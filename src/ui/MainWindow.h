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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWeakPointer>

class ContactUser;
class UserIdentity;
class ChatWidget;
class NotificationWidget;
class IncomingContactRequest;
class OutgoingContactRequest;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

    friend class ChatWidget;

public:
    QAction *actOptions;

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    NotificationWidget *showNotification(const QString &message, QObject *receiver = 0, const char *slot = 0);

public slots:
    void openAddContactDialog(UserIdentity *identity);
    void openTorConfig();

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void contactPageChanged(int page, QObject *userObject);

    /* Incoming contact request notifications */
    void updateContactRequests();
    void showContactRequest();

    /* Outgoing contact request notifications */
    void outgoingRequestAdded(OutgoingContactRequest *request);
    void updateOutgoingRequest(OutgoingContactRequest *request = 0);
    void showRequestInfo();

    /* Tor status notifications */
    void updateTorStatus();
    void enableTorNotification();

private:
    class ContactsView *contactsView;
    class QStackedWidget *chatArea;

    QWeakPointer<NotificationWidget> contactReqNotification;
    QWeakPointer<NotificationWidget> torNotification;
    bool torNotificationEnabled;

    void createActions();
    void createContactsView();
    void createChatArea();
    void addChatWidget(ChatWidget *widget);
};

extern MainWindow *uiMain;

#endif // MAINWINDOW_H
