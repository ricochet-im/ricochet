#include "ContactsView.h"
#include "ContactsModel.h"
#include "ContactItemDelegate.h"
#include "core/ContactUser.h"
#include <QHeaderView>
#include <QMouseEvent>

ContactsView::ContactsView(QWidget *parent)
	: QTreeView(parent)
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
	return activePage.value(activeContact(), InfoPage);
}

void ContactsView::setActiveContact(ContactUser *user)
{
	ContactsModel *cmodel = qobject_cast<ContactsModel*>(model());
	if (!cmodel)
		return;

	QModelIndex index = cmodel->indexOfContact(user);
	if (index.isValid())
		setCurrentIndex(index);
}

void ContactsView::setActivePage(ContactPage page)
{
	setContactPage(activeContact(), page);
}

void ContactsView::setContactPage(ContactUser *user, ContactPage page)
{
	activePage[user] = page;

	ContactUser *activeUser = activeContact();
	if (user == activeUser)
	{
		update(selectionModel()->currentIndex());
		emit activePageChanged(activeUser, page);
	}
}

void ContactsView::mousePressEvent(QMouseEvent *event)
{
	/* Note that the actual press event hasn't happened yet, so current may not be changed. */

	QModelIndex index = indexAt(event->pos());
	if (!index.isValid())
		return;

	bool wasAlreadyActive = (index == currentIndex());

	ContactUser *user = index.data(ContactsModel::ContactUserRole).value<ContactUser*>();
	Q_ASSERT(user);
	if (!user)
		return;

	ContactItemDelegate *delegate = qobject_cast<ContactItemDelegate*>(this->itemDelegate(index));
	if (!delegate)
		return;

	QRect itemRect = visualRect(index);
	QPoint innerPos = event->pos() - itemRect.topLeft();

	ContactPage hitPage;
	if (delegate->pageHitTest(itemRect.size(), innerPos, hitPage))
	{
		setContactPage(user, hitPage);
	}
	else if (wasAlreadyActive)
	{
		/* Clicking on an already selected contact, but not on the page buttons,
		 * causes the page to toggle. */
		hitPage = activePage[user];
		if (hitPage == ChatPage)
			hitPage = InfoPage;
		else
			hitPage = ChatPage;

		setContactPage(user, hitPage);
	}

	QTreeView::mousePressEvent(event);
}

void ContactsView::mouseDoubleClickEvent(QMouseEvent *event)
{
	QModelIndex index = indexAt(event->pos());
	if (!index.isValid())
		return;

	ContactUser *user = index.data(ContactsModel::ContactUserRole).value<ContactUser*>();
	Q_ASSERT(user);
	if (!user)
		return;

	/* Double click goes to chat, always */
	setContactPage(user, ChatPage);

	QTreeView::mouseDoubleClickEvent(event);
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

	ContactUser *user = activeContact();
	if (!activePage.contains(user))
		activePage[user] = InfoPage;

	emit activeContactChanged(user);
	emit activePageChanged(user, activeContactPage());
}
