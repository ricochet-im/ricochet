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
        StatusRole,
        AlertRole /* bool */
    };

    UserIdentity * const identity;

    explicit ContactsModel(UserIdentity *identity, QObject *parent = 0);

    Q_INVOKABLE QModelIndex indexOfContact(ContactUser *user) const;

    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;

    virtual Qt::ItemFlags flags(const QModelIndex &index) const;

    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role);

private slots:
    void updateUser(ContactUser *user = 0);
    void contactAdded(ContactUser *user);
    void contactRemoved(ContactUser *user);

private:
    struct ContactGroup
    {
        QString title;
        int status;
        QList<ContactUser*> contacts;
    };

    QList<ContactGroup> items;

    void populate();
    static bool groupSort(const ContactGroup &g1, const ContactGroup &g2);
};

#endif // CONTACTSMODEL_H
