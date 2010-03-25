#include "ContactItemDelegate.h"
#include <QPainter>
#include <QApplication>

ContactItemDelegate::ContactItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QSize ContactItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(160, 53);
}

void ContactItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt,
								const QModelIndex &index) const
{
	QStyleOptionViewItemV4 ropt = opt;

	QRect r = opt.rect.adjusted(5, 5, -5, -8);

	p->save();

	/* Selection (behind the avatar) */
	ropt.rect = QRect(opt.rect.topLeft() + QPoint(1, 1), QSize(48, 48));
	qApp->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &ropt, p, ropt.widget);

	/* Avatar */
	QPixmap avatar = index.data(Qt::DecorationRole).value<QPixmap>();
	if (avatar.width() > 40 || avatar.height() > 40)
		avatar = avatar.scaled(QSize(40, 40), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	p->drawPixmap(r.x() + (40 - avatar.width()) / 2, r.y() + (40 - avatar.height()) / 2,
				 avatar);
	r.adjust(46, 0, 0, 0);

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
