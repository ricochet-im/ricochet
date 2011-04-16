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

#include "ContactItemDelegate.h"
#include "ContactsViewDelegate.h"
#include "ContactsView.h"
#include "ContactsModel.h"
#include "utils/PaintUtil.h"
#include "core/NicknameValidator.h"
#include "core/ContactUser.h"
#include <QPainter>
#include <QApplication>
#include <QCursor>
#include <QLineEdit>
#include <QFontMetrics>

ContactItemDelegate::ContactItemDelegate(ContactsViewDelegate *cvd)
    : QStyledItemDelegate(cvd), parentDelegate(cvd)
{
    Q_ASSERT(cvd);
}

QSize ContactItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(160, 48);
}

bool ContactItemDelegate::pageHitTest(const QSize &size, const QPoint &point, ContactsView::Page &hitPage) const
{
    /* Point is from an origin of 0,0 in an item of the given size. Perform hit testing on
     * the page switch buttons. */

    QRect chatRect(size.width()-5-16+1, 5-1, 16, 16);
    QRect infoRect(size.width()-5-16+1, size.height()-8-16+1, 16, 16);

    if (chatRect.contains(point))
    {
        hitPage = ContactsView::ContactChatPage;
        return true;
    }
    else if (infoRect.contains(point))
    {
        hitPage = ContactsView::ContactInfoPage;
        return true;
    }
    else
        return false;
}

void ContactItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt,
                                const QModelIndex &index) const
{
    QStyleOptionViewItemV4 ropt = opt;

    QRect r = opt.rect.adjusted(5, 5, -5, -8);

    p->save();

    /* Selection (behind the avatar) */
    ropt.rect.adjust(0, 0, 0, -3);

    SelectionState sst = NoSelectionState;
    if (opt.state & QStyle::State_Selected)
        sst = Selected;
    else if (opt.state & QStyle::State_MouseOver)
        sst = MouseOver;
    else if (index.data(ContactsModel::AlertRole).toBool())
    {
        sst = Alert;
        p->setOpacity(parentDelegate->alertOpacity());
        parentDelegate->startAlertAnimation();
    }

    if (sst != NoSelectionState)
    {
        p->drawPixmap(ropt.rect.topLeft(), customSelectionRect(ropt.rect.size(), sst));

        if (sst == Alert)
            p->setOpacity(1);
    }

    /* Avatar */
    QPixmap avatar = index.data(Qt::DecorationRole).value<QPixmap>();
    QPoint avatarPos = QPoint(r.x() + (35 - avatar.width()) / 2, r.y() + (35 - avatar.height()) / 2);

    if (!avatar.isNull())
    {
        if (avatar.width() > 35 || avatar.height() > 35)
            avatar = avatar.scaled(QSize(35, 35), Qt::KeepAspectRatio, Qt::SmoothTransformation);

        p->drawPixmap(avatarPos, avatar);
    }

    /* Status */
    QPixmap status = index.data(ContactsModel::StatusIndicator).value<QPixmap>();
    if (!status.isNull())
        p->drawPixmap(avatarPos - QPoint(status.width()/2-2, status.height()/2-3), status);

    /* Page switch buttons */
    int textWidth = 0;

    if ((opt.state & QStyle::State_Selected) || (opt.state & QStyle::State_MouseOver))
    {
        ContactsView::Page activePage = parentDelegate->view->activePage();
        bool isActive = (parentDelegate->view->currentIndex() == index);

        /* Chat page */
        QRect iconRect(r.right()-16+1, r.top()-1, 16, 16);
        QPixmap pm;
        if (isActive && activePage == ContactsView::ContactChatPage)
            pm = QPixmap(QLatin1String(":/icons/chat-active.png"));
        else if (iconRect.contains(ropt.widget->mapFromGlobal(QCursor::pos())))
            pm = QPixmap(QLatin1String(":/icons/chat-hover.png"));
        else
            pm = QPixmap(QLatin1String(":/icons/chat-inactive.png"));

        p->drawPixmap(iconRect.topLeft(), pm);

        /* Info page */
        iconRect = QRect(r.right()-16+1, r.bottom()-16+1, 16, 16);
        if (isActive && activePage == ContactsView::ContactInfoPage)
            pm = QPixmap(QLatin1String(":/icons/info-active.png"));
        else if (iconRect.contains(ropt.widget->mapFromGlobal(QCursor::pos())))
            pm = QPixmap(QLatin1String(":/icons/info-hover.png"));
        else
            pm = QPixmap(QLatin1String(":/icons/info-inactive.png"));

        p->drawPixmap(iconRect.topLeft(), pm);

        textWidth -= 14;
    }

    /* Draw nickname */
    r.adjust(41, 0, 0, 0);
    textWidth += r.width();

    QString nickname = index.data().toString();

    QFont nickFont = p->font();
    nickFont.setPixelSize(12);
    p->setFont(nickFont);

    /* Caution: horrifically slow */
    QFontMetrics metrics = p->fontMetrics();
    QRect nickRect = metrics.tightBoundingRect(nickname);

    p->drawText(r.topLeft() + QPoint(0, nickRect.height()+1), nickname);

    /* Draw info text */
    QModelIndex infoIndex(index.model()->index(index.row(), 2, index.parent()));
    QString infoText = infoIndex.data(Qt::DisplayRole).toString();

    QFont infoFont = QFont(QLatin1String("Arial"));
    infoFont.setPixelSize(10);
    infoFont.setStyleHint(QFont::SansSerif);
    p->setFont(infoFont);

    infoText = QFontMetrics(infoFont).elidedText(infoText, Qt::ElideRight, textWidth);

    p->setPen(infoIndex.data(Qt::ForegroundRole).value<QColor>());
    p->drawText(r.bottomLeft() - QPoint(0, 1), infoText);

    p->restore();
}

QWidget *ContactItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);

    /* This is necessary because QTreeView (at least) clips the delegate to the editor area during editing,
     * and we need to render the entire thing. The container widget covers the entire area (which will be
     * the clip), and the actual editor is within that. */
    QWidget *container = new QWidget(parent);
    QLineEdit *widget = new QLineEdit(container);
    container->setFocusProxy(widget);

    QFont nickFont = widget->font();
    nickFont.setPixelSize(12);
    widget->setFont(nickFont);
    widget->setFrame(false);
    widget->setTextMargins(0, 0, 0, 0);
    widget->setContentsMargins(0, 0, 0, 0);

    ContactUser *user = index.data(ContactsModel::PointerRole).value<ContactUser*>();
    Q_ASSERT(user);

    NicknameValidator *validator = new NicknameValidator(widget);
    validator->setValidateUnique(user->identity, user);
    validator->setWidget(widget);
    widget->setValidator(validator);

    connect(widget, SIGNAL(editingFinished()), SLOT(editingFinished()));

    return container;
}

void ContactItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    /* Container is the full size of the index */
    editor->setGeometry(option.rect);

    QWidget *widget = editor->focusProxy();
    if (widget)
    {
        QFontMetrics fm(widget->font());
        widget->setGeometry(QRect(46, 5, option.rect.width()-70, fm.height()));
    }
}

void ContactItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *widget = qobject_cast<QLineEdit*>(editor->focusProxy());
    Q_ASSERT(widget);
    if (!widget)
        return;

    widget->setText(index.data(Qt::EditRole).toString());
    widget->selectAll();
}

void ContactItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *widget = qobject_cast<QLineEdit*>(editor->focusProxy());
    Q_ASSERT(widget);
    if (!widget || !widget->isModified() || !widget->hasAcceptableInput())
        return;

    model->setData(index, widget->text(), Qt::EditRole);
}

void ContactItemDelegate::editingFinished()
{
    QWidget *editor = qobject_cast<QWidget*>(sender());
    Q_ASSERT(editor);
    if (!editor)
        return;

    /* The actual editor is the container */
    editor = editor->parentWidget();

    emit commitData(editor);
    emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
}
