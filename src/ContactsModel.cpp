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
#include "ui/ChatWidget.h"
#include <QImage>
#include <QColor>

/* Used for both identities and contacts, as they share this API */
template<typename T> static inline bool userSort(const T *u1, const T *u2)
{
    int p1 = u1->readSetting("listPosition", -1).toInt();
    int p2 = u2->readSetting("listPosition", -1).toInt();
    if (p1 < 0 && p2 < 0)
        return (u1->uniqueID < u2->uniqueID);
    else if (p2 < 0)
        return true;
    else if (p1 < 0)
        return false;
    else
        return (p1 < p2);
}

ContactsModel::ContactsModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    populate();
}

void ContactsModel::populate()
{
#if QT_VERSION >= 0x040600
    beginResetModel();
#endif

    foreach (QList<ContactUser*> c, contacts)
    {
        for (QList<ContactUser*>::Iterator it = c.begin(); it != c.end(); ++it)
            (*it)->disconnect(this);
    }

    foreach (UserIdentity *i, identities)
    {
        i->disconnect(this);
        i->contacts.disconnect(this);
    }

    contacts.clear();
    identities.clear();

    identities = identityManager->identities();
    qSort(identities.begin(), identities.end(), userSort<UserIdentity>);

    int i = 0;
    for (QList<UserIdentity*>::Iterator it = identities.begin(); it != identities.end(); ++it, ++i)
    {
        connect(&(*it)->contacts, SIGNAL(contactAdded(ContactUser*)), SLOT(contactAdded(ContactUser*)));
        connect(*it, SIGNAL(statusChanged()), SLOT(updateIdentity()));
        connect(*it, SIGNAL(settingsChanged(QString)), SLOT(updateIdentity()));

        QList<ContactUser*> c = (*it)->contacts.contacts();
        qSort(c.begin(), c.end(), userSort<ContactUser>);

        for (QList<ContactUser*>::Iterator it = c.begin(); it != c.end(); ++it)
        {
            /* Duplicated in contactAdded() */
            connect(*it, SIGNAL(connected()), this, SLOT(updateUser()));
            connect(*it, SIGNAL(disconnected()), this, SLOT(updateUser()));
            connect(*it, SIGNAL(statusLineChanged()), this, SLOT(updateUser()));
            connect(*it, SIGNAL(contactDeleted(ContactUser*)), this, SLOT(contactRemoved(ContactUser*)));
        }

        contacts.insert(i, c);
    }

#if QT_VERSION >= 0x040600
    endResetModel();
#else
    reset();
#endif
}

bool ContactsModel::indexIsContact(const QModelIndex &index) const
{
    return index.internalPointer() != 0;
}

QModelIndex ContactsModel::indexOfContact(ContactUser *user) const
{
    if (contacts.isEmpty())
        return QModelIndex();

    int row = contacts[0].indexOf(user);
    if (row < 0)
        return QModelIndex();

    return index(row, 0, index(0, 0));
}

QModelIndex ContactsModel::indexOfIdentity(UserIdentity *user) const
{
    int row = identities.indexOf(user);
    if (row < 0)
        return QModelIndex();

    return index(row, 0);
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

    emit dataChanged(idx, index(idx.row(), columnCount(idx.parent())-1));
}

void ContactsModel::contactAdded(ContactUser *user)
{
    int identityRow = 0;
    if (identities.isEmpty())
        return;

    int i;
    for (i = 0; i < contacts[identityRow].size(); ++i)
        if (!userSort(contacts[identityRow][i], user))
            break;

    beginInsertRows(index(identityRow, 0), i, i);
    contacts[identityRow].insert(i, user);
    endInsertRows();

    saveContactPositions(identityRow);

    connect(user, SIGNAL(connected()), this, SLOT(updateUser()));
    connect(user, SIGNAL(disconnected()), this, SLOT(updateUser()));
    connect(user, SIGNAL(statusLineChanged()), this, SLOT(updateUser()));
    connect(user, SIGNAL(contactDeleted(ContactUser*)), this, SLOT(contactRemoved(ContactUser*)));
}

void ContactsModel::contactRemoved(ContactUser *user)
{
    if (!user && !(user = qobject_cast<ContactUser*>(sender())))
        return;

    QModelIndex idx = indexOfContact(user);
    QModelIndex parent = idx.parent();

    Q_ASSERT(parent.isValid());

    beginRemoveRows(parent, idx.row(), idx.row());
    contacts[parent.row()].removeAt(idx.row());
    endRemoveRows();

    saveContactPositions(parent.row());

    disconnect(user, 0, this, 0);
}

void ContactsModel::updateIdentity(UserIdentity *identity)
{
    if (!identity)
    {
        identity = qobject_cast<UserIdentity*>(sender());
        if (!identity)
            return;
    }

    QModelIndex idx = indexOfIdentity(identity);
    if (!idx.isValid())
    {
        identity->disconnect(this);
        return;
    }

    emit dataChanged(idx, index(idx.row(), columnCount(idx.parent())-1));
}

void ContactsModel::moveRow(int from, int to, const QModelIndex &parent)
{
    if (parent.isValid())
    {
        QList<ContactUser*> &c = contacts[parent.row()];
        if (from < 0 || from >= c.size() || to < 0 || to >= c.size() || from == to)
            return;

#if QT_VERSION >= 0x040600
        bool ok = beginMoveRows(parent, from, from, parent, (to > from) ? (to+1) : to);
        Q_ASSERT(ok);

        c.move(from, to);
        endMoveRows();
#else
        emit layoutAboutToBeChanged();
        c.move(from, to);
        emit layoutChanged();
#endif

        saveContactPositions(parent.row());
    }
    else
    {
        if (from < 0 || from >= identities.size() || to < 0 || to >= identities.size() || from == to)
            return;

#if QT_VERSION >= 0x040600
        bool ok = beginMoveRows(parent, from, from, parent, (to > from) ? (to+1) : to);
        Q_ASSERT(ok);

        identities.move(from, to);
        endMoveRows();
#else
        emit layoutAboutToBeChanged();
        identities.move(from, to);
        emit layoutChanged();
#endif

        saveIdentityPositions();
    }
}

void ContactsModel::saveContactPositions(int identityRow)
{
    /* Update the stored positions */
    int i = 0;
    for (QList<ContactUser*>::Iterator it = contacts[identityRow].begin(), end = contacts[identityRow].end();
         it != end; ++it, ++i)
    {
        if ((*it)->readSetting("listPosition", -1).toInt() != i)
            (*it)->writeSetting("listPosition", i);
    }
}

void ContactsModel::saveIdentityPositions()
{
    for (int i = 0; i < identities.size(); ++i)
    {
        if (identities[i]->readSetting("listPosition", -1).toInt() != i)
            identities[i]->writeSetting("listPosition", i);
    }
}

int ContactsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        if (parent.row() < 0 || parent.row() >= identities.size() || parent.internalPointer())
            return 0;
        return contacts[parent.row()].size();
    }
    else
        return identities.size();
}

int ContactsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

QModelIndex ContactsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        if (parent.row() < 0 || parent.row() >= identities.size() || row < 0 || row >= contacts[parent.row()].size()
            || column < 0 || column >= columnCount(parent))
            return QModelIndex();

        return createIndex(row, column, contacts[parent.row()][row]);
    }
    else
    {
        if (row < 0 || row >= identities.size() || column < 0 || column >= columnCount())
            return QModelIndex();

        return createIndex(row, column);
    }
}

QModelIndex ContactsModel::parent(const QModelIndex &child) const
{
    ContactUser *user = reinterpret_cast<ContactUser*>(child.internalPointer());
    if (!user)
        return QModelIndex();

    // return the real identity..
    return index(0, 0);
}

Qt::DropActions ContactsModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

Qt::ItemFlags ContactsModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags re = 0;

    if (index.isValid())
    {
        re |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
        if (index.column() == 0)
            re |= Qt::ItemIsEditable;
    }

    return re;
}

QVariant ContactsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    ContactUser *user = reinterpret_cast<ContactUser*>(index.internalPointer());

    if (!user)
    {
        /* Identity */
        UserIdentity *identity = identities[index.row()];

        if (role == PointerRole)
            return QVariant::fromValue(identity);
        else if (role == StatusIndicator)
        {
            if (identity->isServiceOnline())
                return QPixmap(QLatin1String(":/icons/status-online.png"));
            else
                return QPixmap(QLatin1String(":/icons/status-offline.png"));
        }

        switch (index.column())
        {
        case 0:
            if (role == Qt::DisplayRole || role == Qt::EditRole)
                return identity->nickname();
            else if (role == Qt::DecorationRole)
                return identity->avatar(TinyAvatar);
            break;
        }
    }
    else
    {
        if (role == PointerRole)
            return QVariant::fromValue(user);
        else if (role == StatusIndicator)
        {
            if (user->isConnected())
                return QPixmap(QLatin1String(":/icons/status-online.png"));
            else
                return QPixmap(QLatin1String(":/icons/status-offline.png"));
        }
        else if (role == AlertRole)
        {
            ChatWidget *chat = ChatWidget::widgetForUser(user, false);
            return (chat && chat->unreadMessages());
        }

        switch (index.column())
        {
        case 0:
            if (role == Qt::DisplayRole || role == Qt::EditRole)
                return user->nickname();
            else if (role == Qt::DecorationRole)
                return user->avatar(TinyAvatar);
            break;
        case 1:
            if (role == Qt::DisplayRole)
                return user->uniqueID;
            break;
        case 2:
            if (role == Qt::DisplayRole)
                return user->statusLine();
            else if (role == Qt::ForegroundRole)
                return user->statusIsError() ? QColor(199, 22, 22) : QColor(Qt::gray);
            break;
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
    if (user)
    {
        validator.setValidateUnique(user->identity, user);
        int pos;
        if (validator.validate(nickname, pos) != QValidator::Acceptable)
            return false;

        user->setNickname(nickname);
    }
    else
    {
        if (index.row() < 0 || index.row() >= identities.size())
            return false;

        UserIdentity *identity = identities[index.row()];

        int pos;
        if (validator.validate(nickname, pos) != QValidator::Acceptable)
            return false;

        identity->setNickname(nickname);
    }

    return true;
}
