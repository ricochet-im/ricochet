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

#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include <QAbstractListModel>
#include <QList>

class ContactUser;

class ContactsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum
    {
        ContactUserRole = Qt::UserRole,
        StatusIndicator
    };

    explicit ContactsModel(QObject *parent = 0);

    QModelIndex indexOfContact(ContactUser *user) const;

    void moveRow(int from, int to);

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private slots:
    void updateUser(ContactUser *user = 0);
    void contactAdded(ContactUser *user);

private:
    QList<ContactUser*> contacts;

    void populate();
    void savePositions();
};

#endif // CONTACTSMODEL_H
