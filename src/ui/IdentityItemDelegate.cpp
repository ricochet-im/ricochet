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

#include "IdentityItemDelegate.h"
#include "utils/PaintUtil.h"
#include "ContactsModel.h"
#include <QPainter>
#include <QLinearGradient>

IdentityItemDelegate::IdentityItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

static const int xMargin = 5;
static const int yMargin = 2;
static const int topShadow = 3, bottomShadow = 4;

QSize IdentityItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont font = option.font;
    font.setPointSize(12);
    QFontMetrics fm(font);

    return QSize(160, fm.height() + (2*yMargin) + bottomShadow + (index.row() ? topShadow : 0));
}

void IdentityItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    p->save();

    /* Exclude shadow area */
    QRect r = opt.rect.adjusted(0, (index.row() ? topShadow : 0), 0, -bottomShadow);

    /* Selection */
    QPixmap selection = customSelectionRect(r.size(), opt.state);

    if (!selection.isNull())
    {
        p->drawPixmap(r.topLeft(), selection);
    }
    else
    {
        QLinearGradient gradient(0, 0, 0, 1);
        gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        gradient.setColorAt(0, QColor(253, 253, 253));
        gradient.setColorAt(1, QColor(231, 231, 231));

        p->fillRect(opt.rect, QBrush(gradient));

        /* Shadowing */
        const int left = opt.rect.left();
        const int right = opt.rect.right();
        const int bottom = opt.rect.bottom();
        const int top = opt.rect.top();

        int shd[4] = { 216, 232, 243, 251 };

        p->setPen(QColor(shd[0], shd[0], shd[0]));
        p->drawLine(QPoint(left, bottom - 3), QPoint(right, bottom - 3));
        p->setPen(QColor(shd[1], shd[1], shd[1]));
        p->drawLine(QPoint(left, bottom - 2), QPoint(right, bottom - 2));
        p->setPen(QColor(shd[2], shd[2], shd[2]));
        p->drawLine(QPoint(left, bottom - 1), QPoint(right, bottom - 1));
        p->setPen(QColor(shd[3], shd[3], shd[3]));
        p->drawLine(QPoint(left, bottom), QPoint(right, bottom));

        if (index.row() != 0)
        {
            for (int i = 0; i < 3; ++i)
                shd[i] = qMin(shd[i]+10, 255);

            p->setPen(QColor(shd[0], shd[0], shd[0]));
            p->drawLine(QPoint(left, top + 2), QPoint(right, top + 2));
            p->setPen(QColor(shd[1], shd[1], shd[1]));
            p->drawLine(QPoint(left, top + 1), QPoint(right, top + 1));
            p->setPen(QColor(shd[2], shd[2], shd[2]));
            p->drawLine(QPoint(left, top), QPoint(right, top));
        }
    }

    /* Exclude margins from r */
    r.adjust(xMargin, yMargin, -xMargin, -yMargin);

    /* Name */
    QFont nameFont = opt.font;
    nameFont.setPointSize(12);
    p->setFont(nameFont);
    p->setPen(QColor(0x00, 0x66, 0xaa));

    p->drawText(r, Qt::AlignCenter, index.data().toString());

    /* Status */
    QPixmap status = index.data(ContactsModel::StatusIndicator).value<QPixmap>();
    p->drawPixmap((qreal)r.x(), (qreal)r.y() + 1 + ((r.height() - status.height())/(qreal)2), status);

    p->restore();
}
