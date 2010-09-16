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

#include "main.h"
#include <QPainter>
#include <QPixmap>
#include <QLinearGradient>
#include <QPixmapCache>
#include <QStyle>

QPixmap customSelectionRect(const QSize &size, QStyle::State state)
{
    QString cacheKey;
    cacheKey.reserve(32);
    cacheKey += QLatin1String("cselr-");
    cacheKey += QString::number(size.width()) + QLatin1Char('x') + QString::number(size.height());

    if (state & QStyle::State_Selected)
        cacheKey += QLatin1String("-s");
    else if (state & QStyle::State_MouseOver)
        cacheKey += QLatin1String("-h");
    else
        return QPixmap();

    QPixmap re;
    if (QPixmapCache::find(cacheKey, re))
        return re;

    re = QPixmap(size);
    re.fill(Qt::transparent);

    QPainter p(&re);

    QLinearGradient gradient(0, 0, 0, 1);
    gradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    if (state & QStyle::State_Selected)
    {
        gradient.setColorAt(0, QColor(242, 248, 255));
        gradient.setColorAt(1, QColor(211, 232, 255));
        p.setPen(QPen(QColor(114, 180, 211)));
    }
    else if (state & QStyle::State_MouseOver)
    {
        gradient.setColorAt(0, QColor(250, 250, 255));
        gradient.setColorAt(1, QColor(235, 244, 255));
        p.setPen(QPen(QColor(160, 201, 220)));
    }

    p.setBrush(QBrush(gradient));

    p.drawRoundedRect(QRect(1, 1, size.width() - 2, size.height() - 2), 3, 3);

    p.end();

    QPixmapCache::insert(cacheKey, re);
    return re;
}

QPixmap shadowedAvatar(const QPixmap &avatar)
{
    /* 6x6 */
    static const quint32 topLeftData[] =
    {
        0x1000000u, 0x2000000u, 0x4000000u, 0x7000000u, 0x9000000u, 0xa000000u,
        0x2000000u, 0x6000000u, 0x0u, 0x0u, 0x0u, 0x0u,
        0x4000000u, 0xc000000u, 0x0u, 0x0u, 0x0u, 0x0u,
        0x7000000u, 0x14000000u, 0x0u, 0x0u, 0x0u, 0x0u,
        0x9000000u, 0x1a000000u, 0x0u, 0x0u, 0x0u, 0x0u,
        0xa000000u, 0x1e000000u, 0x0u, 0x0u, 0x0u, 0x0u
    };
    /* 6x6 */
    static const quint32 topRightData[] =
    {
        0xa000000u, 0x9000000u, 0x7000000u, 0x4000000u, 0x2000000u, 0x1000000u,
        0x0u, 0x0u, 0x14000000u, 0xc000000u, 0x6000000u, 0x2000000u,
        0x0u, 0x0u, 0x28000000u, 0x18000000u, 0xc000000u, 0x4000000u,
        0x0u, 0x0u, 0x42000000u, 0x28000000u, 0x14000000u, 0x7000000u,
        0x0u, 0x0u, 0x55000000u, 0x33000000u, 0x1a000000u, 0x9000000u,
        0x0u, 0x0u, 0x62000000u, 0x3b000000u, 0x1e000000u, 0xa000000u
    };
    /* 6x6 */
    static const quint32 bottomRightData[] =
    {
        0x0u, 0x0u, 0x62000000u, 0x3b000000u, 0x1e000000u, 0xa000000u,
        0x80000000u, 0x6f000000u, 0x55000000u, 0x33000000u, 0x1a000000u, 0x9000000u,
        0x62000000u, 0x55000000u, 0x42000000u, 0x28000000u, 0x14000000u, 0x7000000u,
        0x3b000000u, 0x33000000u, 0x28000000u, 0x18000000u, 0xc000000u, 0x4000000u,
        0x1e000000u, 0x1a000000u, 0x14000000u, 0xc000000u, 0x6000000u, 0x2000000u,
        0xa000000u, 0x9000000u, 0x7000000u, 0x4000000u, 0x2000000u, 0x1000000u
    };
    /* 6x6 */
    static const quint32 bottomLeftData[] =
    {
        0xa000000u, 0x1e000000u, 0x0u, 0x0u, 0x0u, 0x0u,
        0x9000000u, 0x1a000000u, 0x33000000u, 0x55000000u, 0x6f000000u, 0x80000000u,
        0x7000000u, 0x14000000u, 0x28000000u, 0x42000000u, 0x55000000u, 0x62000000u,
        0x4000000u, 0xc000000u, 0x18000000u, 0x28000000u, 0x33000000u, 0x3b000000u,
        0x2000000u, 0x6000000u, 0xc000000u, 0x14000000u, 0x1a000000u, 0x1e000000u,
        0x1000000u, 0x2000000u, 0x4000000u, 0x7000000u, 0x9000000u, 0xa000000u
    };
    /* 1x5 */
    static const quint32 bottomData[] =
    {
        0x88000000u,
        0x69000000u,
        0x3f000000u,
        0x20000000u,
        0xb000000u
    };
    /* 4x1 */
    static const quint32 rightData[] = { 0x69000000u, 0x3f000000u, 0x20000000u, 0x0b000000u };

    QImage topLeft(reinterpret_cast<const uchar*>(topLeftData), 6, 6, QImage::Format_ARGB32);
    QImage topRight(reinterpret_cast<const uchar*>(topRightData), 6, 6, QImage::Format_ARGB32);
    QImage bottomRight(reinterpret_cast<const uchar*>(bottomRightData), 6, 6, QImage::Format_ARGB32);
    QImage bottomLeft(reinterpret_cast<const uchar*>(bottomLeftData), 6, 6, QImage::Format_ARGB32);
    QImage right(reinterpret_cast<const uchar*>(rightData), 4, 1, QImage::Format_ARGB32);
    QImage bottom(reinterpret_cast<const uchar*>(bottomData), 1, 5, QImage::Format_ARGB32);

    /* Shadowed image */
    QPixmap shadowAvatar(avatar.width() + 6, avatar.height() + 6);
    shadowAvatar.fill(qRgb(240, 240, 240));
    QPainter p(&shadowAvatar);

    QRect imageRect(2, 1, avatar.width(), avatar.height());

    /* Draw avatar */
    p.drawPixmap(imageRect.topLeft(), avatar);

    /* Draw top-left corner */
    p.drawImage(0, 0, topLeft);

    /* Draw top */
    p.setPen(QColor(0, 0, 0, 11));
    p.drawLine(6, 0, imageRect.right() - 2, 0);

    /* Draw left */
    p.drawLine(0, 6, 0, imageRect.bottom() - 1);

    p.setPen(QColor(0, 0, 0, 32));
    p.drawLine(1, 6, 1, imageRect.bottom() - 1);

    /* Draw top-right corner */
    p.drawImage(imageRect.right() - 1, 0, topRight);

    /* Draw right-side repeat */
    p.drawTiledPixmap(QRect(imageRect.right()+1, 6, 4, imageRect.height() - 6), QPixmap::fromImage(right));

    /* Draw bottom-right corner */
    p.drawImage(imageRect.right() - 1, imageRect.bottom(), bottomRight);

    /* Draw bottom repeat */
    p.drawTiledPixmap(QRect(6, imageRect.bottom()+1, avatar.width() - 6, 5), QPixmap::fromImage(bottom));

    /* Draw bottom-left corner */
    p.drawImage(0, imageRect.bottom(), bottomLeft);

    p.end();

    return shadowAvatar;
}
