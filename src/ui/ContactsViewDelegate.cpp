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

#include "ContactsViewDelegate.h"
#include "ContactsModel.h"

ContactsViewDelegate::ContactsViewDelegate(ContactsView *view)
    : QAbstractItemDelegate(view), contactDelegate(view)
{
    contactDelegate.setParent(0);
}

bool ContactsViewDelegate::indexIsContact(const QModelIndex &index) const
{
    Q_ASSERT(qobject_cast<const ContactsModel*>(index.model()));
    const ContactsModel *model = reinterpret_cast<const ContactsModel*>(index.model());
    return model->indexIsContact(index);
}

bool ContactsViewDelegate::pageHitTest(const QModelIndex &index, const QSize &size, const QPoint &point,
                                       ContactsView::Page &hitPage) const
{
    if (!indexIsContact(index))
        return false;

    return contactDelegate.pageHitTest(size, point, hitPage);
}

QSize ContactsViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (indexIsContact(index))
        return contactDelegate.sizeHint(option, index);
    else
        return identityDelegate.sizeHint(option, index);
}

void ContactsViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    if (indexIsContact(index))
        contactDelegate.paint(painter, option, index);
    else
        identityDelegate.paint(painter, option, index);
}


