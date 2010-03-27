#ifndef CONTACTSVIEW_H
#define CONTACTSVIEW_H

#include <QTreeView>

class ContactsView : public QTreeView
{
	Q_OBJECT
	Q_DISABLE_COPY(ContactsView)

public:
	explicit ContactsView(QWidget *parent = 0);

protected:
	virtual void mouseMoveEvent(QMouseEvent *event);
};

#endif // CONTACTSVIEW_H
