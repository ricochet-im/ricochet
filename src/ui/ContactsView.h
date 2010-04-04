#ifndef CONTACTSVIEW_H
#define CONTACTSVIEW_H

#include "MainWindow.h"
#include <QTreeView>
#include <QHash>

class ContactUser;

class ContactsView : public QTreeView
{
	Q_OBJECT
	Q_DISABLE_COPY(ContactsView)

public:
	explicit ContactsView(QWidget *parent = 0);

	ContactUser *activeContact() const { return pActiveContact; }
	ContactPage activeContactPage() const;

public slots:
	void setActiveContact(ContactUser *user);
	void setActivePage(ContactPage page);

signals:
	void activeContactChanged(ContactUser *user);
	void activePageChanged(ContactUser *user, ContactPage page);

protected:
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseReleaseEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
	void contactSelected(const QModelIndex &current);

private:
	QHash<ContactUser*,ContactPage> activePage;
	ContactUser *pActiveContact;
	QModelIndex dragIndex;
	bool clickSetCurrent, clickItemMoved;

	void setContactPage(ContactUser *user, ContactPage page);
};

#endif // CONTACTSVIEW_H
