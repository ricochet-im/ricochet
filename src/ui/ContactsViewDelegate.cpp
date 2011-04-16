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
#ifdef Q_OS_MAC
    if (option.state & QStyle::State_MouseOver)
    {
        /* Work around a Qt bug that leaves the MouseOver state set after the mouse has
        * left the widget (as of Qt 4.7.2). See ContactsView::leaveEvent. */
        const QWidget *widget = QStyleOptionViewItemV4(option).widget;
        if (widget && !QRect(widget->mapToGlobal(QPoint(0,0)),widget->size())
                       .contains(QCursor::pos()))
        {
            const_cast<QStyleOptionViewItem&>(option).state &= ~QStyle::State_MouseOver;
        }
    }
#endif

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
