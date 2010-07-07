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

#ifndef CONTACTSVIEW_H
#define CONTACTSVIEW_H

#include "MainWindow.h"
#include <QTreeView>
#include <QHash>

class ContactUser;
class UserIdentity;

class ContactsView : public QTreeView
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactsView)

public:
    enum Page
    {
        ContactChatPage,
        ContactInfoPage,
        IdentityInfoPage
    };

    explicit ContactsView(QWidget *parent = 0);

    Page activePage() const { return pActivePage; }
    QObject *activeObject() const;
    ContactUser *activeContact() const;
    UserIdentity *activeIdentity() const;

public slots:
    void setActiveContact(ContactUser *user);
    void setActiveIdentity(UserIdentity *identity);
    void setActivePage(Page page);

    void showContactInfo(ContactUser *user);
    void showContactChat(ContactUser *user);

signals:
    void activePageChanged(int page, QObject *userObject);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
    void currentChanged(const QModelIndex &current);

private:
    QHash<ContactUser*,Page> savedContactPage;
    Page pActivePage;
    QModelIndex dragIndex;
    bool clickSetCurrent, clickItemMoved;

    void setContactPage(ContactUser *user, Page page);
};

#endif // CONTACTSVIEW_H
