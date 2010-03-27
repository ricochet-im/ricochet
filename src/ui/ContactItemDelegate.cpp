#include "ContactItemDelegate.h"
#include <QPainter>
#include <QApplication>
#include <QCursor>

ContactItemDelegate::ContactItemDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QSize ContactItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	qDebug("Size hint");
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
	QString infoText("11 months ago");

	QFont infoFont = QFont("Arial", 8);
	p->setFont(infoFont);

	p->setPen(Qt::gray);
	p->drawText(r.bottomLeft() - QPoint(0, 1), infoText);

	/* Icons */
	if ((opt.state & QStyle::State_Selected) || (opt.state & QStyle::State_MouseOver))
	{
		QRect iconRect(r.right()-16+1, r.top()-1, 16, 16);
		QPixmap pm;
		//if (opt.state & QStyle::State_Selected)
			//pm = QPixmap(":/icons/chat-active.png");
		if (iconRect.contains(ropt.widget->mapFromGlobal(QCursor::pos())))
			pm = QPixmap(":/icons/chat-hover.png");
		else
			pm = QPixmap(":/icons/chat-inactive.png");

		p->drawPixmap(iconRect.topLeft(), pm);

		iconRect = QRect(r.right()-16+1, r.bottom()-16+1, 16, 16);
		if (opt.state & QStyle::State_Selected)
			pm = QPixmap(":/icons/info-active.png");
		else if (iconRect.contains(ropt.widget->mapFromGlobal(QCursor::pos())))
			pm = QPixmap(":/icons/info-hover.png");
		else
			pm = QPixmap(":/icons/info-inactive.png");

		p->drawPixmap(iconRect.topLeft(), pm);
	}

	p->restore();
}
