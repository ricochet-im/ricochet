#include "ContactItemDelegate.h"
#include <QPainter>

ContactItemDelegate::ContactItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QSize ContactItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(160, 45);
}

void ContactItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt,
								const QModelIndex &index) const
{
	QRect r = opt.rect.adjusted(5, 5, -5, 0);

	p->save();

	p->fillRect(QRect(r.topLeft(), QSize(40, 40)), Qt::green);
	r.adjust(44, 0, 0, 0);

	/* Draw nickname */
	QString nickname = index.data().toString();

	QFont nickFont = QFont("Calibri", 11);
	p->setFont(nickFont);

	/* Caution: horrifically slow */
	QFontMetrics metrics = p->fontMetrics();
	QRect nickRect = metrics.tightBoundingRect(nickname);

	p->drawText(r.topLeft() + QPoint(0, nickRect.height()+1), nickname);

	/* Draw info text */
	QString infoText("Last seen: 3 weeks ago");

	QFont infoFont = QFont("Arial", 8);
	p->setFont(infoFont);

	p->setPen(Qt::gray);
	p->drawText(r.bottomLeft() - QPoint(0, 1), infoText);

	p->restore();
}
