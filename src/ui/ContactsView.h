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

	ContactUser *activeContact() const;
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
	virtual void mouseDoubleClickEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
	void contactSelected(const QModelIndex &current);

private:
	QHash<ContactUser*,ContactPage> activePage;
	QModelIndex dragIndex;
	bool blockSelectionChanges;

	void setContactPage(ContactUser *user, ContactPage page);
};

#endif // CONTACTSVIEW_H
