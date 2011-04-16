/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
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
    virtual void leaveEvent(QEvent *event);
    virtual void contextMenuEvent(QContextMenuEvent *event);

    virtual void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

    virtual void keyPressEvent(QKeyEvent *event);

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
