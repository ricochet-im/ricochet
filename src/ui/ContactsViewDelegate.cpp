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

#include "ContactsViewDelegate.h"
#include "ContactsModel.h"

ContactsViewDelegate::ContactsViewDelegate(ContactsView *view)
    : QAbstractItemDelegate(view), contactDelegate(view)
{
    contactDelegate.setParent(0);

    QAbstractItemDelegate *d = &contactDelegate;
    do
    {
        connect(d, SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),
                SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)));
        connect(d, SIGNAL(commitData(QWidget*)), SIGNAL(commitData(QWidget*)));
        connect(d, SIGNAL(sizeHintChanged(QModelIndex)), SIGNAL(sizeHintChanged(QModelIndex)));

        if (d == &contactDelegate)
            d = &identityDelegate;
        else
            d = 0;
    } while (d);
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
    return delegateForIndex(index)->sizeHint(option, index);
}

void ContactsViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    delegateForIndex(index)->paint(painter, option, index);
}

void ContactsViewDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                                const QModelIndex &index) const
{
    delegateForIndex(index)->updateEditorGeometry(editor, option, index);
}

QWidget *ContactsViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return delegateForIndex(index)->createEditor(parent, option, index);
}

bool ContactsViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                       const QModelIndex &index)
{
    return delegateForIndex(index)->editorEvent(event, model, option, index);
}

void ContactsViewDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    delegateForIndex(index)->setEditorData(editor, index);
}

void ContactsViewDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    delegateForIndex(index)->setModelData(editor, model, index);
}
