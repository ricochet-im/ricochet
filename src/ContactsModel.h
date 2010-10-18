/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include <QAbstractItemModel>
#include <QList>

class UserIdentity;
class ContactUser;

class ContactsModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactsModel)

public:
    enum
    {
        PointerRole = Qt::UserRole,
        StatusIndicator,
        AlertRole /* bool */
    };

    explicit ContactsModel(QObject *parent = 0);

    bool indexIsContact(const QModelIndex &index) const;

    QModelIndex indexOfContact(ContactUser *user) const;
    QModelIndex indexOfIdentity(UserIdentity *user) const;

    void moveRow(int from, int to, const QModelIndex &parent = QModelIndex());

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual Qt::DropActions supportedDropActions() const;
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

private slots:
    void updateUser(ContactUser *user = 0);
    void contactAdded(ContactUser *user);
    void contactRemoved(ContactUser *user);

    void updateIdentity(UserIdentity *identity = 0);

private:
    QList<UserIdentity*> identities;
    QList<QList<ContactUser*> > contacts;

    void populate();
    void saveIdentityPositions();
    void saveContactPositions(int identityRow);
};

#endif // CONTACTSMODEL_H
