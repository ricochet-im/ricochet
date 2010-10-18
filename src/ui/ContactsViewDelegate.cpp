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

#include "ContactsViewDelegate.h"
#include "ContactsModel.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

ContactsViewDelegate::ContactsViewDelegate(ContactsView *v)
    : QAbstractItemDelegate(v), view(v), contactDelegate(this), m_alertAnimation(0), m_alertOpacity(0)
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

void ContactsViewDelegate::startAlertAnimation()
{
    if (!m_alertAnimation)
    {
        QSequentialAnimationGroup *ag = new QSequentialAnimationGroup(this);
        m_alertAnimation = ag;

        QPropertyAnimation *aIn = new QPropertyAnimation(this, "alertOpacity");
        aIn->setEndValue(qreal(1));
        aIn->setEasingCurve(QEasingCurve::OutQuad);
        aIn->setDuration(750);

        QPropertyAnimation *aOut = new QPropertyAnimation(this, "alertOpacity");
        aOut->setEndValue(qreal(0.2));
        aOut->setEasingCurve(QEasingCurve::InQuad);
        aOut->setDuration(750);

        ag->addAnimation(aIn);
        ag->addPause(150);
        ag->addAnimation(aOut);
        ag->setLoopCount(-1);
        ag->start();
    }
}

static inline bool updateAlerts(QAbstractItemView *view, QAbstractItemModel *model, const QModelIndex &parent)
{
    bool re = false;
    for (int r = 0, rc = model->rowCount(parent); r < rc; ++r)
    {
        /* Assuming that only column 0 needs to be updated */
        QModelIndex index = model->index(r, 0, parent);
        if (index.data(ContactsModel::AlertRole).toBool())
        {
            view->update(index);
            re = true;
        }

        if (model->hasChildren(index) && updateAlerts(view, model, index))
            re = true;
    }

    return re;
}

void ContactsViewDelegate::setAlertOpacity(qreal alertOpacity)
{
    if (alertOpacity == m_alertOpacity)
        return;

    m_alertOpacity = alertOpacity;

    if (!updateAlerts(view, view->model(), view->rootIndex()))
    {
        m_alertAnimation->stop();
        m_alertAnimation->deleteLater();
        m_alertAnimation = 0;
    }
}
