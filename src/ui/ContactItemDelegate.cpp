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

#include "ContactItemDelegate.h"
#include "ContactsView.h"
#include "ContactsModel.h"
#include "utils/PaintUtil.h"
#include <QPainter>
#include <QApplication>
#include <QCursor>

ContactItemDelegate::ContactItemDelegate(ContactsView *view)
    : QStyledItemDelegate(view), contactsView(view)
{
    Q_ASSERT(view);
}

QSize ContactItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
        Q_UNUSED(option);
        Q_UNUSED(index);
    return QSize(160, 48);
}

bool ContactItemDelegate::pageHitTest(const QSize &size, const QPoint &point, ContactPage &hitPage)
{
    /* Point is from an origin of 0,0 in an item of the given size. Perform hit testing on
     * the page switch buttons. */

    QRect chatRect(size.width()-5-16+1, 5-1, 16, 16);
    QRect infoRect(size.width()-5-16+1, size.height()-8-16+1, 16, 16);

    if (chatRect.contains(point))
    {
        hitPage = ChatPage;
        return true;
    }
    else if (infoRect.contains(point))
    {
        hitPage = InfoPage;
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
    p->drawPixmap(ropt.rect.topLeft(), customSelectionRect(ropt.rect.size(), ropt.state));

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
        ContactPage activePage = contactsView->activeContactPage();
        bool isActive = (contactsView->currentIndex() == index);

        /* Chat page */
        QRect iconRect(r.right()-16+1, r.top()-1, 16, 16);
        QPixmap pm;
        if (isActive && activePage == ChatPage)
            pm = QPixmap(QLatin1String(":/icons/chat-active.png"));
        else if (iconRect.contains(ropt.widget->mapFromGlobal(QCursor::pos())))
            pm = QPixmap(QLatin1String(":/icons/chat-hover.png"));
        else
            pm = QPixmap(QLatin1String(":/icons/chat-inactive.png"));

        p->drawPixmap(iconRect.topLeft(), pm);

        /* Info page */
        iconRect = QRect(r.right()-16+1, r.bottom()-16+1, 16, 16);
        if (isActive && activePage == InfoPage)
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

    QFont nickFont = QFont(QLatin1String("Calibri"), 11);
    p->setFont(nickFont);

    /* Caution: horrifically slow */
    QFontMetrics metrics = p->fontMetrics();
    QRect nickRect = metrics.tightBoundingRect(nickname);

    p->drawText(r.topLeft() + QPoint(0, nickRect.height()+1), nickname);

    /* Draw info text */
    QString infoText = index.model()->index(index.row(), 2, index.parent()).data(Qt::DisplayRole).toString();

    QFont infoFont = QFont(QLatin1String("Arial"), 8);
    p->setFont(infoFont);

    infoText = QFontMetrics(infoFont).elidedText(infoText, Qt::ElideRight, textWidth);

    p->setPen(Qt::gray);
    p->drawText(r.bottomLeft() - QPoint(0, 1), infoText);

    p->restore();
}
