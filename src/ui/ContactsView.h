#ifndef CONTACTSVIEW_H
#define CONTACTSVIEW_H

#include "MainWindow.h"
#include <QTreeView>

class ContactUser;

class ContactsView : public QTreeView
{
	Q_OBJECT
	Q_DISABLE_COPY(ContactsView)

public:
	explicit ContactsView(QWidget *parent = 0);

	ContactUser *activeContact() const;
	ContactPage activeContactPage() const;

signals:
	void activeContactChanged(ContactUser *user);
	void activePageChanged(ContactUser *user, ContactPage page);

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
	void contactSelected(const QModelIndex &current);

private:
	ContactPage activePage;
};

#endif // CONTACTSVIEW_H
