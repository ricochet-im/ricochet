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

#include "main.h"
#include "HomeContactWidget.h"
#include "utils/PaintUtil.h"
#include <QMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QPropertyAnimation>

HomeContactWidget::HomeContactWidget(QWidget *parent)
    : QWidget(parent), pSelected(false), pIconOffset(0)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setToolTip(tr("Your Information", "Tooltip for the 'Home' button in the contact list"));
}

void HomeContactWidget::setSelected(bool value)
{
    if (pSelected == value)
        return;

    pSelected = value;

    emit selectionChanged(pSelected);

    if (pSelected)
        emit selected();
    else
        emit deselected();

    if (!isVisible())
        return;

    QPropertyAnimation *ani = new QPropertyAnimation(this, QByteArray("iconOffset"), this);
    ani->setEndValue(0);

    int maxOffset = ((width() - 16) / 2 - 5);

    if (pSelected)
    {
        pIconOffset -= maxOffset;
        ani->setDuration(200);
        ani->setEasingCurve(QEasingCurve::OutBack);
    }
    else
    {
        pIconOffset += maxOffset;
        ani->setDuration(250);
        ani->setEasingCurve(QEasingCurve::InCubic);
    }

    update();
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void HomeContactWidget::setIconOffset(int offset)
{
    if (pIconOffset == offset)
        return;

    pIconOffset = offset;
    update();
}

QSize HomeContactWidget::sizeHint() const
{
    return QSize(24, 24);
}

void HomeContactWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton)
    {
        QWidget::mousePressEvent(event);
        return;
    }

    setSelected();
}

void HomeContactWidget::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (isSelected())
        return;

    //update();

    QPropertyAnimation *ani = new QPropertyAnimation(this, QByteArray("iconOffset"), this);
    ani->setEndValue(((width() - 16) / 2 - 5));
    ani->setDuration(200);
    ani->setEasingCurve(QEasingCurve::OutBack);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void HomeContactWidget::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    if (isSelected())
        return;

    //update();

    QPropertyAnimation *ani = new QPropertyAnimation(this, QByteArray("iconOffset"), this);
    ani->setEndValue(0);
    ani->setDuration(250);
    ani->setEasingCurve(QEasingCurve::InCubic);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void HomeContactWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter p(this);
    QRect r = rect();

    QPixmap icon(QLatin1String(":/icons/home.png"));

    int xpos = (r.width() - icon.width());

    bool mouseOver = QRect(mapToGlobal(QPoint(0,0)), size()).contains(QCursor::pos());

    if (isSelected())
    {
        p.drawPixmap(r.topLeft(), customSelectionRect(r.size(), QStyle::State_Selected));
        xpos = qRound(xpos / 2.0);
    }
    else if (mouseOver)
    {
        p.drawPixmap(r.topLeft(), customSelectionRect(r.size(), QStyle::State_MouseOver));
        xpos -= 5;
    }
    else
    {
        p.setOpacity(0.5);
        xpos -= 5;
    }

    xpos -= iconOffset();

    p.drawPixmap(xpos, (r.height() - icon.height() - 4), icon);
}
