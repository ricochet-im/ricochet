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

#include "ContactsView.h"
#include "ContactsModel.h"
#include "ContactItemDelegate.h"
#include "core/ContactUser.h"
#include "ui/ChatWidget.h"
#include <QHeaderView>
#include <QMouseEvent>

ContactsView::ContactsView(QWidget *parent)
    : QTreeView(parent), clickSetCurrent(false), clickItemMoved(false)
{
    setRootIsDecorated(false);
    setHeaderHidden(true);
    setFrameStyle(QFrame::NoFrame);
    setModel(new ContactsModel(this));
    setItemDelegate(new ContactItemDelegate(this));
    setMouseTracking(true);
    setDragEnabled(true);
    setAcceptDrops(true);

    QHeaderView *head = header();
    for (int i = 1; i < head->count(); ++i)
        head->hideSection(i);
    head->setResizeMode(0, QHeaderView::Stretch);

    connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this,
            SLOT(contactSelected(QModelIndex)));

    if (model()->rowCount())
        selectionModel()->setCurrentIndex(model()->index(0, 0), QItemSelectionModel::Select);
}

ContactPage ContactsView::activeContactPage() const
{
    return activePage.value(activeContact(), InfoPage);
}

void ContactsView::setActiveContact(ContactUser *user)
{
    ContactsModel *cmodel = qobject_cast<ContactsModel*>(model());
    if (!cmodel)
        return;

    QModelIndex index = cmodel->indexOfContact(user);

    pActiveContact = user;
    if (pActiveContact && !activePage.contains(pActiveContact))
        activePage[pActiveContact] = InfoPage;

    setCurrentIndex(index);

    emit activeContactChanged(pActiveContact);
    if (pActiveContact)
        emit activePageChanged(pActiveContact, activeContactPage());
}

void ContactsView::setActivePage(ContactPage page)
{
    setContactPage(activeContact(), page);
}

void ContactsView::contactSelected(const QModelIndex &current)
{
    ContactUser *user = current.data(ContactsModel::ContactUserRole).value<ContactUser*>();
    if (user != activeContact())
        setActiveContact(user);
}

void ContactsView::setContactPage(ContactUser *user, ContactPage page)
{
    activePage[user] = page;

    ContactUser *activeUser = activeContact();
    if (user == activeUser)
    {
        update(selectionModel()->currentIndex());
        emit activePageChanged(activeUser, page);
    }
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
    ContactUser *user = index.data(ContactsModel::ContactUserRole).value<ContactUser*>();

    if (user)
    {
        ContactItemDelegate *delegate = qobject_cast<ContactItemDelegate*>(this->itemDelegate(index));
        Q_ASSERT(delegate);

        if (delegate)
        {
            /* Hit test for the page switch buttons and set the contact's page if one is matched */
            QRect itemRect = visualRect(index);
            QPoint innerPos = event->pos() - itemRect.topLeft();
            ContactPage hitPage;

            if (delegate->pageHitTest(itemRect.size(), innerPos, hitPage))
                setContactPage(user, hitPage);
        }

        /* If there are any unread messages from the user, always go to chat */
        ChatWidget *chat = ChatWidget::widgetForUser(user, false);
        if (chat && chat->unreadMessages() > 0)
            setContactPage(user, ChatPage);
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

    ContactItemDelegate *delegate = qobject_cast<ContactItemDelegate*>(this->itemDelegate(index));
    if (!delegate)
    {
        QTreeView::mouseReleaseEvent(event);
        return;
    }

    QRect itemRect = visualRect(index);
    QPoint innerPos = event->pos() - itemRect.topLeft();

    ContactPage hitPage;
    if (delegate->pageHitTest(itemRect.size(), innerPos, hitPage))
    {
        setContactPage(user, hitPage);
    }
    else
    {
        /* Clicking on an already selected contact, but not on the page buttons,
         * causes the page to toggle. */
        hitPage = activePage[user];
        if (hitPage == ChatPage)
            hitPage = InfoPage;
        else
            hitPage = ChatPage;

        setContactPage(user, hitPage);
    }

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
        if (event->buttons() & Qt::LeftButton && dragIndex.isValid() && dragIndex != index)
        {
            ContactsModel *cmodel = qobject_cast<ContactsModel*>(model());
            Q_ASSERT(cmodel);

            cmodel->moveRow(dragIndex.row(), index.row());
            dragIndex = cmodel->index(index.row(), 0);
            Q_ASSERT(indexAt(event->pos()) == dragIndex);

            clickItemMoved = true;
        }
    }

    QTreeView::mouseMoveEvent(event);
}
