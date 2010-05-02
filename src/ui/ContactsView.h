/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

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
