#include "ContactItemDelegate.h"
#include <QPainter>
#include <QApplication>

ContactItemDelegate::ContactItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QSize ContactItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(160, 48);
}

void ContactItemDelegate::paint(QPainter *p, const QStyleOptionViewItem &opt,
								const QModelIndex &index) const
{
	QStyleOptionViewItemV4 ropt = opt;

	QRect r = opt.rect.adjusted(5, 5, -5, -8);

	p->save();

	/* Selection (behind the avatar) */
	ropt.rect.adjust(0, 0, 0, -3);
	ropt.state |= QStyle::State_Active;
	qApp->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &ropt, p, ropt.widget);

	/* Avatar */
	QPixmap avatar = index.data(Qt::DecorationRole).value<QPixmap>();
	if (avatar.width() > 35 || avatar.height() > 35)
		avatar = avatar.scaled(QSize(35, 35), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	p->drawPixmap(r.x() + (35 - avatar.width()) / 2, r.y() + (35 - avatar.height()) / 2,
				 avatar);
	r.adjust(41, 0, 0, 0);

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
