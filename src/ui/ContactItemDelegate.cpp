#include "ContactItemDelegate.h"
#include "ContactsView.h"
#include <QPainter>
#include <QApplication>
#include <QCursor>

ContactItemDelegate::ContactItemDelegate(ContactsView *view)
	: QStyledItemDelegate(view), contactsView(view)
{
	Q_ASSERT(view);
}

QSize ContactItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(160, 48);
}

bool ContactItemDelegate::pageHitTest(const QSize &size, const QPoint &point, ContactPage &hitPage)
{
	/* Point is from an origin of 0,0 in an item of the given size. Perform hit testing on
	 * the page switch buttons. */

	QRect chatRect(size.width()-5-16+1, 5-1, 16, 16);
	QRect infoRect(size.width()-5-16+1, size.height()-8-16+1, 16, 16);

	if (chatRect.contains(point))
	{
		hitPage = ChatPage;
		return true;
	}
	else if (infoRect.contains(point))
	{
		hitPage = InfoPage;
		return true;
	}
	else
		return false;
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
	QPoint avatarPos = QPoint(r.x() + (35 - avatar.width()) / 2, r.y() + (35 - avatar.height()) / 2);

	if (!avatar.isNull())
	{
		if (avatar.width() > 35 || avatar.height() > 35)
			avatar = avatar.scaled(QSize(35, 35), Qt::KeepAspectRatio, Qt::SmoothTransformation);

		p->drawPixmap(avatarPos, avatar);
	}

	/* Status */
	QPixmap status(":/icons/status-online.png");
	p->drawPixmap(avatarPos - QPoint(status.width()/2-1, status.height()/2-1), status);

	/* Draw nickname */
	r.adjust(41, 0, 0, 0);

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

	/* Page switch buttons */
	if ((opt.state & QStyle::State_Selected) || (opt.state & QStyle::State_MouseOver))
	{
		ContactPage activePage = contactsView->activeContactPage();
		bool isActive = (contactsView->currentIndex() == index);

		/* Chat page */
		QRect iconRect(r.right()-16+1, r.top()-1, 16, 16);
		QPixmap pm;
		if (isActive && activePage == ChatPage)
			pm = QPixmap(":/icons/chat-active.png");
		else if (iconRect.contains(ropt.widget->mapFromGlobal(QCursor::pos())))
			pm = QPixmap(":/icons/chat-hover.png");
		else
			pm = QPixmap(":/icons/chat-inactive.png");

		p->drawPixmap(iconRect.topLeft(), pm);

		/* Info page */
		iconRect = QRect(r.right()-16+1, r.bottom()-16+1, 16, 16);
		if (isActive && activePage == InfoPage)
			pm = QPixmap(":/icons/info-active.png");
		else if (iconRect.contains(ropt.widget->mapFromGlobal(QCursor::pos())))
			pm = QPixmap(":/icons/info-hover.png");
		else
			pm = QPixmap(":/icons/info-inactive.png");

		p->drawPixmap(iconRect.topLeft(), pm);
	}

	p->restore();
}
