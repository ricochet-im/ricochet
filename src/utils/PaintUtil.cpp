#include "main.h"
#include <QPainter>
#include <QPixmap>
#include <QLinearGradient>
#include <QPixmapCache>
#include <QStyle>

QPixmap customSelectionRect(const QSize &size, QStyle::State state)
{
	if (!(state & QStyle::State_Selected))
		return QPixmap();

	QString cacheKey;
	cacheKey.reserve(32);
	cacheKey += QLatin1String("cselr-");
	cacheKey += QString::number(size.width()) + QChar('x') + QString::number(size.height());

	QPixmap re;
	if (QPixmapCache::find(cacheKey, &re))
		return re;

	re = QPixmap(size);
	re.fill(Qt::transparent);

	QPainter p(&re);

	QLinearGradient gradient(0, 0, 0, 1);
	gradient.setCoordinateMode(QGradient::ObjectBoundingMode);
	gradient.setColorAt(0, QColor(242, 248, 255));
	gradient.setColorAt(1, QColor(211, 232, 255));

	p.setBrush(QBrush(gradient));
	p.setPen(QPen(QColor(114, 180, 211)));

	p.drawRoundedRect(QRect(1, 1, size.width() - 2, size.height() - 2), 3, 3);

	p.end();

	QPixmapCache::insert(cacheKey, re);
	return re;
}
