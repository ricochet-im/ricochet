#include "ContactsView.h"
#include "ContactsModel.h"
#include "ContactItemDelegate.h"
#include <QHeaderView>
#include <QMouseEvent>

ContactsView::ContactsView(QWidget *parent) :
    QTreeView(parent)
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
}

void ContactsView::mouseMoveEvent(QMouseEvent *event)
{
	update(indexAt(event->pos()));
}
