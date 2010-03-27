#include "ContactsView.h"
#include "ContactsModel.h"
#include "ContactItemDelegate.h"
#include "core/ContactUser.h"
#include <QHeaderView>
#include <QMouseEvent>

ContactsView::ContactsView(QWidget *parent)
	: QTreeView(parent), activePage(InfoPage)
{
	setRootIsDecorated(false);
	setHeaderHidden(true);
	setFrameStyle(QFrame::NoFrame);
	setModel(new ContactsModel(this));
	setItemDelegate(new ContactItemDelegate(this));
	setMouseTracking(true);

	QHeaderView *head = header();
	for (int i = 1; i < head->count(); ++i)
		head->hideSection(i);
	head->setResizeMode(0, QHeaderView::Stretch);

	connect(selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this,
			SLOT(contactSelected(QModelIndex)));

	if (model()->rowCount())
		selectionModel()->setCurrentIndex(model()->index(0, 0), QItemSelectionModel::Select);
}

ContactUser *ContactsView::activeContact() const
{
	QModelIndex index = selectionModel()->currentIndex();
	if (!index.isValid())
		return 0;

	return index.data(ContactsModel::ContactUserRole).value<ContactUser*>();
}

ContactPage ContactsView::activeContactPage() const
{
	return activePage;
}

void ContactsView::mousePressEvent(QMouseEvent *event)
{
	QTreeView::mousePressEvent(event);

	QModelIndex index = indexAt(event->pos());
	if (!index.isValid())
		return;

	ContactItemDelegate *delegate = qobject_cast<ContactItemDelegate*>(this->itemDelegate(index));
	if (!delegate)
		return;

	QRect itemRect = visualRect(index);
	QPoint innerPos = event->pos() - itemRect.topLeft();

	if (delegate->pageHitTest(itemRect.size(), innerPos, activePage))
	{
		update(index);
		emit activePageChanged(activeContact(), activeContactPage());
	}
}

void ContactsView::mouseMoveEvent(QMouseEvent *event)
{
	QModelIndex index = indexAt(event->pos());
	if (index.isValid())
		update(index);

	QTreeView::mouseMoveEvent(event);
}

void ContactsView::contactSelected(const QModelIndex &current)
{
	Q_UNUSED(current);
	emit activeContactChanged(activeContact());
	emit activePageChanged(activeContact(), activeContactPage());
}
