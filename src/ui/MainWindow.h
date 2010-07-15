/* Torsion - http://github.com/special/torsion
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
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
    QList<NotificationWidget*> notifications() const { return m_notifications; }

public slots:
    void openAddContactDialog(UserIdentity *identity);
    void openTorConfig();

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void contactPageChanged(int page, QObject *userObject);

    void notificationRemoved(QObject *object);

    /* Incoming contact request notifications */
    void updateContactRequests();
    void showContactRequest();

    /* Outgoing contact request notifications */
    void outgoingRequestAdded(OutgoingContactRequest *request);
    void updateOutgoingRequest(OutgoingContactRequest *request = 0);
    void showRequestInfo();
    void clearRequestNotification(ContactUser *user);

    /* Tor status notifications */
    void updateTorStatus();
    void enableTorNotification();

private:
    class ContactsView *contactsView;
    class QStackedWidget *chatArea;

    QList<NotificationWidget*> m_notifications;
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
