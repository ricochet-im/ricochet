/* Torsion - http://torsionim.org/
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

#include "ContactsView.h"
#include "ContactsModel.h"
#include "ContactsViewDelegate.h"
#include "core/ContactUser.h"
#include "core/UserIdentity.h"
#include "ui/ChatWidget.h"
#include <QHeaderView>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>

ContactsView::ContactsView(QWidget *parent)
    : QTreeView(parent), clickSetCurrent(false), clickItemMoved(false)
{
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setHeaderHidden(true);
    setFrameStyle(QFrame::NoFrame);
    setModel(new ContactsModel(this));
    setItemDelegate(new ContactsViewDelegate(this));
    setMouseTracking(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setIndentation(0);
    setContextMenuPolicy(Qt::DefaultContextMenu);

    QHeaderView *head = header();
    for (int i = 1; i < head->count(); ++i)
        head->hideSection(i);
    head->setResizeMode(0, QHeaderView::Stretch);

    connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this,
            SLOT(currentChanged(QModelIndex)));

    if (model()->rowCount())
        selectionModel()->setCurrentIndex(model()->index(0, 0), QItemSelectionModel::Select);

    expandAll();
}

void ContactsView::setActiveContact(ContactUser *user)
{
    Q_ASSERT(user);

    ContactsModel *cmodel = static_cast<ContactsModel*>(model());
    QModelIndex index = cmodel->indexOfContact(user);
    if (!index.isValid())
        return;

    if (!savedContactPage.contains(user))
        savedContactPage[user] = ContactInfoPage;

    setCurrentIndex(index);
}

void ContactsView::setActiveIdentity(UserIdentity *identity)
{
    Q_ASSERT(identity);

    ContactsModel *cmodel = static_cast<ContactsModel*>(model());
    QModelIndex index = cmodel->indexOfIdentity(identity);
    if (!index.isValid())
        return;

    setCurrentIndex(index);
}

void ContactsView::setActivePage(Page page)
{
    ContactUser *user;
    if (page != activePage() && page < IdentityInfoPage && (user = activeContact()))
    {
        savedContactPage[user] = pActivePage = page;
        update(selectionModel()->currentIndex());
        emit activePageChanged(pActivePage, user);
    }
}

QObject *ContactsView::activeObject() const
{
    QObject *re = activeContact();
    if (!re)
        re = activeIdentity();
    return re;
}

ContactUser *ContactsView::activeContact() const
{
    return currentIndex().data(ContactsModel::PointerRole).value<ContactUser*>();
}

UserIdentity *ContactsView::activeIdentity() const
{
    return currentIndex().data(ContactsModel::PointerRole).value<UserIdentity*>();
}

void ContactsView::currentChanged(const QModelIndex &current)
{
    /* React to the active object (contact or identity) changing */
    QVariant vObject = current.data(ContactsModel::PointerRole);
    if (vObject.canConvert<ContactUser*>())
    {
        ContactUser *user = vObject.value<ContactUser*>();
        pActivePage = savedContactPage.value(user, ContactInfoPage);
        savedContactPage.insert(user, pActivePage);
        emit activePageChanged(pActivePage, user);
    }
    else if (vObject.canConvert<UserIdentity*>())
    {
        UserIdentity *identity = vObject.value<UserIdentity*>();
        pActivePage = IdentityInfoPage;
        emit activePageChanged(pActivePage, identity);
    }
}

void ContactsView::setContactPage(ContactUser *user, Page page)
{
    if (user == activeContact())
        setActivePage(page);
    else
        savedContactPage[user] = page;
}

void ContactsView::showContactChat(ContactUser *user)
{
    setContactPage(user, ContactChatPage);
    if (user != activeContact())
        setActiveContact(user);
}

void ContactsView::showContactInfo(ContactUser *user)
{
    setContactPage(user, ContactInfoPage);
    if (user != activeContact())
        setActiveContact(user);
}

void ContactsView::mousePressEvent(QMouseEvent *event)
{
    /* Only clicks that change the current contact are handled as press events; clicks on an
     * already selected contact (to change page, etc) are handled within the release event, which
     * enables more natural behavior when dragging. */
    clickSetCurrent = clickItemMoved = false;
    
    /* Index that was clicked on */
    QModelIndex index = indexAt(event->pos());
    dragIndex = index;

    /* Contact user for that index */
    ContactUser *user = index.data(ContactsModel::PointerRole).value<ContactUser*>();

    if (user)
    {
        ContactsViewDelegate *delegate = qobject_cast<ContactsViewDelegate*>(this->itemDelegate(index));
        Q_ASSERT(delegate);

        if (delegate)
        {
            /* Hit test for the page switch buttons and set the contact's page if one is matched */
            QRect itemRect = visualRect(index);
            QPoint innerPos = event->pos() - itemRect.topLeft();
            Page hitPage;

            if (delegate->pageHitTest(index, itemRect.size(), innerPos, hitPage))
                setContactPage(user, hitPage);
        }

        /* If there are any unread messages from the user, always go to chat */
        ChatWidget *chat = ChatWidget::widgetForUser(user, false);
        if (chat && chat->unreadMessages() > 0)
            setContactPage(user, ContactChatPage);
    }

    /* Index that was selected before the event */
    QModelIndex oldCurrent = currentIndex();

    QTreeView::mousePressEvent(event);

    /* New selected index; if this is different, set a flag to indicate that for the release event */
    if (oldCurrent != currentIndex())
        clickSetCurrent = true;
}

void ContactsView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton || clickSetCurrent || clickItemMoved)
    {
        clickSetCurrent = clickItemMoved = false;
        QTreeView::mouseReleaseEvent(event);
        return;
    }

    /* The index we're dealing with is the same as the index that was clicked on.
     * dragIndex provides a shortcut to that. */
    QModelIndex index = dragIndex;
    if (!index.isValid())
    {
        QTreeView::mouseReleaseEvent(event);
        return;
    }

    /* Reset click state */
    clickSetCurrent = clickItemMoved = false;
    dragIndex = QModelIndex();

    ContactUser *user = activeContact();
    if (!user)
    {
        QTreeView::mouseReleaseEvent(event);
        return;
    }

    ContactsViewDelegate *delegate = qobject_cast<ContactsViewDelegate*>(this->itemDelegate(index));
    if (!delegate)
    {
        QTreeView::mouseReleaseEvent(event);
        return;
    }

    QRect itemRect = visualRect(index);
    QPoint innerPos = event->pos() - itemRect.topLeft();

    Page hitPage;
    if (!delegate->pageHitTest(index, itemRect.size(), innerPos, hitPage))
    {
        /* Clicking on an already selected contact, but not on the page buttons,
         * causes the page to toggle. */
        hitPage = savedContactPage[user];
        if (hitPage == ContactChatPage)
            hitPage = ContactInfoPage;
        else
            hitPage = ContactChatPage;
    }

    setContactPage(user, hitPage);

    QTreeView::mouseReleaseEvent(event);
}

void ContactsView::mouseMoveEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid())
    {
        /* Update the index for hover styles */
        update(index);

        /* Drag and drop */
        if (event->buttons() & Qt::LeftButton && dragIndex.isValid() && dragIndex != index
            && dragIndex.parent() == index.parent())
        {
            Q_ASSERT(qobject_cast<ContactsModel*>(model()));
            ContactsModel *cmodel = reinterpret_cast<ContactsModel*>(model());

            cmodel->moveRow(dragIndex.row(), index.row(), index.parent());
            dragIndex = cmodel->index(index.row(), 0, index.parent());
            Q_ASSERT(indexAt(event->pos()) == dragIndex);

            clickItemMoved = true;
        }
    }

    QTreeView::mouseMoveEvent(event);
}

void ContactsView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QModelIndex current = currentIndex();
    if (current.parent() == parent && current.row() >= start && current.row() <= end)
    {
        /* Currently selected contact will be removed; select the associated identity instead */
        setCurrentIndex(parent);
    }

    QTreeView::rowsAboutToBeRemoved(parent, start, end);
}

void ContactsView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);

    ContactUser *user;
    if ((user = activeContact()))
    {
        QAction *actChat = menu.addAction(tr("Send IM"));
        QAction *actInfo = menu.addAction(tr("View profile"));
        menu.addSeparator();
        QAction *actCopyID = menu.addAction(QIcon(QLatin1String(":/icons/vcard.png")), tr("Copy contact ID"));
        QAction *actChangeName = menu.addAction(tr("Change nickname"));
        menu.addSeparator();
        QAction *actRemove = menu.addAction(QIcon(QLatin1String(":/icons/cross.png")), tr("Remove contact"));

        menu.setDefaultAction((activePage() == ContactInfoPage) ? actChat : actInfo);

        QAction *action = menu.exec(event->globalPos());

        if (action == actChat)
        {
            setActivePage(ContactChatPage);
        }
        else if (action == actInfo)
        {
            setActivePage(ContactInfoPage);
        }
        else if (action == actCopyID)
        {
            qApp->clipboard()->setText(user->contactID());
        }
        else if (action == actChangeName)
        {
            edit(currentIndex());
        }
        else if (action == actRemove)
        {
            uiMain->uiRemoveContact(user);
        }
    }
}
