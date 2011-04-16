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

#include "NotificationWidget.h"
#include <QTextLayout>
#include <QPainter>
#include <QLinearGradient>

#if QT_VERSION >= 0x040600
#include <QPropertyAnimation>
#endif

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
#if QT_VERSION >= 0x040600
    setMaximumHeight(0);

    QPropertyAnimation *ani = new QPropertyAnimation(this, QByteArray("maximumHeight"));
    ani->setStartValue(0);
    ani->setEndValue(qMax(heightForWidth(width()), sizeHint().height()));
    ani->setDuration(500);
    ani->setEasingCurve(QEasingCurve::InCubic);
    ani->start(QAbstractAnimation::DeleteWhenStopped);
#endif
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
    qreal height = 0, leading = QFontMetrics(layout.font()).leading();
    for (;;)
    {
        QTextLine line = layout.createLine();
        if (!line.isValid())
            break;

        line.setLineWidth(rect.width());
        line.setPosition(QPointF(rect.x(), rect.y() + height));
        height += line.height() + leading;
    }
    layout.endLayout();
}
