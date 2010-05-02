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
