#ifndef CONTACTITEMDELEGATE_H
#define CONTACTITEMDELEGATE_H

#include "MainWindow.h"
#include <QStyledItemDelegate>

class ContactsView;

class ContactItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit ContactItemDelegate(ContactsView *view);

	bool pageHitTest(const QSize &size, const QPoint &point, ContactPage &hitPage);

	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;

protected:
	ContactsView *contactsView;
};

#endif // CONTACTITEMDELEGATE_H
