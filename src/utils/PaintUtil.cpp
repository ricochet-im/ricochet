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
    if (QPixmapCache::find(cacheKey, &re))
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
