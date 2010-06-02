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

#include "NotificationWidget.h"
#include <QTextLayout>
#include <QPainter>
#include <QLinearGradient>
#include <QPropertyAnimation>

static int wMargin = 2, hMargin = 6;

NotificationWidget::NotificationWidget(QWidget *parent)
    : QWidget(parent)
{
    visualSetup();
}

NotificationWidget::NotificationWidget(const QString &message, QWidget *parent)
    : QWidget(parent)
{
    setMessage(message);
    visualSetup();
}

void NotificationWidget::visualSetup()
{
    QSizePolicy policy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    policy.setHeightForWidth(true);
    setSizePolicy(policy);

    /* Background */
    QLinearGradient gradient(0, 0, 0, 1);
    gradient.setColorAt(0, QColor(255, 249, 154));
    gradient.setColorAt(1, QColor(255, 230, 142));
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    backgroundBrush = QBrush(gradient);
    setAutoFillBackground(false);

    /* Font */
    QFont f = font();
    f.setPointSize(f.pointSize() * 1.2);
    setFont(f);

    /* Cursor */
    setCursor(Qt::PointingHandCursor);
}

void NotificationWidget::showAnimated()
{
    setMaximumHeight(0);

    QPropertyAnimation *ani = new QPropertyAnimation(this, QByteArray("maximumHeight"));
    ani->setStartValue(0);
    ani->setEndValue(qMax(heightForWidth(width()), sizeHint().height()));
    ani->setDuration(500);
    ani->setEasingCurve(QEasingCurve::InCubic);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
}

void NotificationWidget::closeNotification()
{
    hide();
    deleteLater();
}

void NotificationWidget::setMessage(const QString &message)
{
    m_message = message;

    updateGeometry();
    update();
}

void NotificationWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        e->accept();
        emit clicked();
    }
    else
        QWidget::mousePressEvent(e);
}

QSize NotificationWidget::sizeHint() const
{
    if (m_message.isEmpty())
        return QSize();

    QFontMetrics metrics(font());
    return metrics.size(0, m_message) + QSize(wMargin * 2, hMargin * 2);
}

int NotificationWidget::heightForWidth(int width) const
{
    QTextLayout textLayout;
    layoutMessage(textLayout, QRect(0, 0, width-(wMargin*2), 0));

    return textLayout.boundingRect().size().toSize().height() + (hMargin * 2);
}

void NotificationWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter p(this);
    QRect r = rect();

    p.fillRect(r, backgroundBrush);

    p.setPen(Qt::black);

    QTextLayout textLayout;
    textLayout.setCacheEnabled(true);
    layoutMessage(textLayout, r.adjusted(wMargin, hMargin, -wMargin, -hMargin));
    textLayout.draw(&p, r.topLeft());

    p.setPen(Qt::gray);
    p.drawLine(r.bottomLeft(), r.bottomRight());
}

void NotificationWidget::layoutMessage(QTextLayout &layout, const QRect &rect) const
{
    layout.setFont(font());
    layout.setText(m_message);

    QTextOption option = layout.textOption();
    option.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout.setTextOption(option);

    layout.beginLayout();
    qreal height = 0;
    for (;;)
    {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(rect.width());
        line.setLeadingIncluded(true);
        line.setPosition(QPointF(rect.x(), rect.y() + height));
        height += line.height();
    }
    layout.endLayout();
}
