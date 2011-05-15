/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ContactsModel.h"
#include "core/IdentityManager.h"
#include "core/ContactsManager.h"
#include "core/NicknameValidator.h"
#include <QImage>
#include <QColor>
#include <QDebug>

inline bool contactSort(const ContactUser *c1, const ContactUser *c2)
{
    return c1->nickname().localeAwareCompare(c2->nickname()) < 0;
}

inline bool ContactsModel::groupSort(const ContactGroup &g1, const ContactGroup &g2)
{
    return g1.status < g2.status;
}

ContactsModel::ContactsModel(UserIdentity *i, QObject *parent)
    : QAbstractItemModel(parent), identity(i)
{
    QHash<int,QByteArray> roles;
    roles[Qt::DisplayRole] = "nickname";
    roles[Qt::DecorationRole] = "avatarPath";
    roles[PointerRole] = "contact";
    roles[StatusRole] = "status";
    setRoleNames(roles);

    connect(&identity->contacts, SIGNAL(contactAdded(ContactUser*)), SLOT(contactAdded(ContactUser*)));

    populate();
}

void ContactsModel::populate()
{
    beginResetModel();

    foreach (const ContactGroup &g, items)
    {
        for (QList<ContactUser*>::ConstIterator it = g.contacts.begin(); it != g.contacts.end(); ++it)
            (*it)->disconnect(this);
    }

    items.clear();

    QList<ContactUser*> c = identity->contacts.contacts();

    for (QList<ContactUser*>::Iterator it = c.begin(); it != c.end(); ++it)
    {
        int status = (*it)->status();

        int groupRow = -1;
        for (int i = 0; i < items.size(); ++i)
        {
            if (items[i].status == status)
            {
                groupRow = i;
                break;
            }
        }

        if (groupRow < 0)
        {
            ContactGroup g;
            g.title = ContactUser::statusString((ContactUser::Status)status);
            g.status = status;

            QList<ContactGroup>::Iterator gp = qLowerBound(items.begin(), items.end(), g, groupSort);
            gp = items.insert(gp, g);

            groupRow = gp - items.begin();
        }

        connect(*it, SIGNAL(statusChanged()), SLOT(updateUser()));
        connect(*it, SIGNAL(contactDeleted(ContactUser*)), SLOT(contactRemoved(ContactUser*)));

        QList<ContactUser*> &list = items[groupRow].contacts;
        QList<ContactUser*>::Iterator lp = qLowerBound(list.begin(), list.end(), *it, contactSort);
        list.insert(lp, *it);
    }

    endResetModel();
}

QModelIndex ContactsModel::indexOfContact(ContactUser *user) const
{
    for (int i = 0; i < items.size(); ++i)
    {
        int r = items[i].contacts.indexOf(user);
        if (r >= 0)
            return index(r, 0, index(i, 0));
    }

    return QModelIndex();
}

void ContactsModel::updateUser(ContactUser *user)
{
    if (!user)
    {
        user = qobject_cast<ContactUser*>(sender());
        if (!user)
            return;
    }

    QModelIndex idx = indexOfContact(user);
    if (!idx.isValid())
    {
        user->disconnect(this);
        return;
    }

    QModelIndex parent = idx.parent();
    int groupRow = parent.row();

    if (user->status() != items[groupRow].status)
    {
        int newGroupRow = -1;
        for (int i = 0; i < items.size(); ++i)
        {
            if (items[i].status == user->status())
            {
                newGroupRow = i;
                break;
            }
        }

        if (newGroupRow < 0)
        {
            ContactGroup g;
            g.title = user->statusString();
            g.status = user->status();

            QList<ContactGroup>::Iterator gp = qLowerBound(items.begin(), items.end(), g, groupSort);
            newGroupRow = gp - items.begin();

            qDebug() << "newGroupRow" << newGroupRow;

            beginInsertRows(QModelIndex(), newGroupRow, newGroupRow);
            gp = items.insert(gp, g);
            endInsertRows();

            idx = indexOfContact(user);
            parent = idx.parent();
            groupRow = parent.row();
        }

        QList<ContactUser*> &list = items[newGroupRow].contacts;
        QList<ContactUser*>::Iterator lp = qLowerBound(list.begin(), list.end(), user, contactSort);
        int newRow = lp - list.begin();

        beginMoveRows(parent, idx.row(), idx.row(), index(newGroupRow, 0), newRow);
        list.insert(lp, user);
        qDebug() << "removing" << idx.row() << "from grouprow" << groupRow << "after moving to" << newGroupRow
                 << "old" << items[groupRow].contacts[idx.row()]->nickname() << "new" << user->nickname();
        items[groupRow].contacts.removeAt(idx.row());
        endMoveRows();

        /* XXX remove empty old group */

        for (int i = 0; i < items.size(); ++i)
        {
            qDebug() << "idx" << i << "status" << items[i].status << ":";
            for (int j = 0; j < items[i].contacts.size(); ++j)
            {
                qDebug() << "    idx" << j << "status" << items[i].contacts[j]->status()
                         << "nickname" << items[i].contacts[j]->nickname();
            }
        }
    }
    else
        emit dataChanged(idx, idx);
}

void ContactsModel::contactAdded(ContactUser *user)
{
    Q_ASSERT(!indexOfContact(user).isValid());

    connect(user, SIGNAL(statusChanged()), SLOT(updateUser()));
    connect(user, SIGNAL(contactDeleted(ContactUser*)), SLOT(contactRemoved(ContactUser*)));

    int status = user->status();

    int groupRow = -1;
    for (int i = 0; i < items.size(); ++i)
    {
        if (items[i].status == status)
        {
            groupRow = i;
            break;
        }
    }

    if (groupRow < 0)
    {
        ContactGroup g;
        g.title = ContactUser::statusString((ContactUser::Status)status);
        g.status = status;
        g.contacts.append(user);

        QList<ContactGroup>::Iterator gp = qLowerBound(items.begin(), items.end(), g, groupSort);
        groupRow = gp - items.begin();

        beginInsertRows(QModelIndex(), groupRow, groupRow);
        items.insert(gp, g);
        endInsertRows();
    }
    else
    {
        QList<ContactUser*> &list = items[groupRow].contacts;
        QList<ContactUser*>::Iterator lp = qLowerBound(list.begin(), list.end(), user, contactSort);
        int row = lp - list.begin();

        beginInsertRows(index(groupRow, 0), row, row);
        list.insert(lp, user);
        endInsertRows();
    }
}

void ContactsModel::contactRemoved(ContactUser *user)
{
    if (!user && !(user = qobject_cast<ContactUser*>(sender())))
        return;

    QModelIndex idx = indexOfContact(user);
    QModelIndex parent = idx.parent();
    Q_ASSERT(parent.isValid());
    int groupRow = parent.row();

    if (items[groupRow].contacts.size() == 1)
    {
        /* Remove the entire group */
        beginRemoveRows(QModelIndex(), groupRow, groupRow);
        items.removeAt(groupRow);
        endRemoveRows();
    }
    else
    {
        beginRemoveRows(parent, idx.row(), idx.row());
        items[groupRow].contacts.removeAt(idx.row());
        endRemoveRows();
    }

    disconnect(user, 0, this, 0);
}

int ContactsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        if (parent.internalPointer())
            return 0;
        return items[parent.row()].contacts.size();
    }
    else
        return items.size();
}

int ContactsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QModelIndex ContactsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        if (parent.internalPointer() || row >= items[parent.row()].contacts.size())
            return QModelIndex();
        return createIndex(row, column, items[parent.row()].contacts[row]);
    }
    else
    {
        if (row >= items.size())
            return QModelIndex();
        return createIndex(row, column);
    }
}

QModelIndex ContactsModel::parent(const QModelIndex &child) const
{
    ContactUser *user = reinterpret_cast<ContactUser*>(child.internalPointer());
    if (!user)
        return QModelIndex();

    for (int i = 0; i < items.size(); ++i)
    {
        if (items[i].contacts.contains(user))
            return index(i, 0);
    }

    return QModelIndex();
}

Qt::ItemFlags ContactsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags re = Qt::ItemIsEnabled;

    if (index.internalPointer())
        re |= Qt::ItemIsSelectable | Qt::ItemIsEditable;

    return re;
}

QVariant ContactsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ContactUser *user = reinterpret_cast<ContactUser*>(index.internalPointer());

    if (!user)
    {
        /* Group */
        const ContactGroup &group = items[index.row()];

        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return group.title;
        case StatusRole:
            return group.status;
        }
    }
    else
    {
        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return user->nickname();
        case Qt::DecorationRole:
            return user->avatar(TinyAvatar);
        case PointerRole:
            return QVariant::fromValue(user);
        case StatusRole:
            return user->status();
        }
    }

    return QVariant();
}

bool ContactsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.column() != 0 || role != Qt::EditRole)
        return false;

    QString nickname = value.toString();
    NicknameValidator validator;

    ContactUser *user = reinterpret_cast<ContactUser*>(index.internalPointer());
    if (!user)
        return false;

    validator.setValidateUnique(user->identity, user);
    int pos;
    if (validator.validate(nickname, pos) != QValidator::Acceptable)
        return false;

    user->setNickname(nickname);
    return true;
}
