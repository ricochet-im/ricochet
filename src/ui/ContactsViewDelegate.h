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

#ifndef CONTACTSVIEWDELEGATE_H
#define CONTACTSVIEWDELEGATE_H

#include <QAbstractItemDelegate>
#include "ContactItemDelegate.h"
#include "IdentityItemDelegate.h"
#include "ContactsView.h"

class QAbstractAnimation;

class ContactsViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactsViewDelegate)

    Q_PROPERTY(qreal alertOpacity READ alertOpacity WRITE setAlertOpacity)

public:
    ContactsView * const view;

    explicit ContactsViewDelegate(ContactsView *view);

    bool indexIsContact(const QModelIndex &index) const;

    bool pageHitTest(const QModelIndex &index, const QSize &size, const QPoint &point, ContactsView::Page &hitPage) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    qreal alertOpacity() const { return m_alertOpacity; }
    void setAlertOpacity(qreal alertOpacity);

    void startAlertAnimation();

private:
    ContactItemDelegate contactDelegate;
    IdentityItemDelegate identityDelegate;

    QAbstractAnimation *m_alertAnimation;
    qreal m_alertOpacity;

    QAbstractItemDelegate *delegateForIndex(const QModelIndex &index) const;
};

inline QAbstractItemDelegate *ContactsViewDelegate::delegateForIndex(const QModelIndex &index) const
{
    if (indexIsContact(index))
        return const_cast<ContactItemDelegate*>(&contactDelegate);
    else
        return const_cast<IdentityItemDelegate*>(&identityDelegate);
}

#endif // CONTACTSVIEWDELEGATE_H
