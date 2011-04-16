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
    font.setPixelSize(16);
    QFontMetrics fm(font);

    return QSize(160, fm.height() + (2*yMargin) + bottomShadow + (index.row() ? topShadow : 0));
}

void IdentityItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    p->save();

    /* Exclude shadow area */
    QRect r = opt.rect.adjusted(0, (index.row() ? topShadow : 0), 0, -bottomShadow);

    /* Selection */
    if (opt.state & (QStyle::State_Selected | QStyle::State_MouseOver))
    {
        SelectionState sst = (opt.state & QStyle::State_Selected) ? Selected : MouseOver;
        p->drawPixmap(r.topLeft(), customSelectionRect(r.size(), sst));
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

        for (int i = 0; i < 4; ++i)
        {
            p->setPen(QColor(shd[i], shd[i], shd[i]));
            p->drawLine(QPoint(left, bottom - (3-i)), QPoint(right, bottom - (3-i)));
        }

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
    nameFont.setPixelSize(16);
    p->setFont(nameFont);
    p->setPen(QColor(0x00, 0x66, 0xaa));

    p->drawText(r, Qt::AlignCenter, index.data().toString());

    /* Status */
    QPixmap status = index.data(ContactsModel::StatusIndicator).value<QPixmap>();
    p->drawPixmap((qreal)r.x(), (qreal)r.y() + 1 + ((r.height() - status.height())/(qreal)2), status);

    p->restore();
}
