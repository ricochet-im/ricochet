#include "ContactsModel.h"
#include "core/ContactsManager.h"
#include <QImage>
#include <QColor>

ContactsModel::ContactsModel(QObject *parent) :
    QAbstractListModel(parent)
{
	populate();
}

void ContactsModel::populate()
{
	beginResetModel();

	for (QList<ContactUser*>::Iterator it = contacts.begin(); it != contacts.end(); ++it)
		(*it)->disconnect(this);

	contacts = contactsManager->contacts();

	for (QList<ContactUser*>::Iterator it = contacts.begin(); it != contacts.end(); ++it)
	{
		connect(*it, SIGNAL(connected()), this, SLOT(updateUser()));
		connect(*it, SIGNAL(disconnected()), this, SLOT(updateUser()));
	}

	endResetModel();
}

QModelIndex ContactsModel::indexOfContact(ContactUser *user) const
{
	int row = contacts.indexOf(user);
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

	int row = contacts.indexOf(user);
	if (row < 0)
	{
		user->disconnect(this);
		return;
	}

	emit dataChanged(index(row, 0), index(row, columnCount()-1));
}

int ContactsModel::rowCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	else
		return contacts.size();
}

int ContactsModel::columnCount(const QModelIndex &parent) const
{
	if (parent.isValid())
		return 0;
	else
		return 2;
}

QVariant ContactsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	ContactUser *user = contacts[index.row()];

	if (role == ContactUserRole)
		return QVariant::fromValue(user);
	else if (role == StatusIndicator)
	{
		if (user->isConnected())
			return QPixmap(":/icons/status-online.png");
		else
			return QPixmap(":/icons/status-offline.png");
	}

	switch (index.column())
	{
	case 0:
		if (role == Qt::DisplayRole)
			return user->nickname();
		else if (role == Qt::DecorationRole)
			return user->avatar(ContactUser::TinyAvatar);
		break;
	case 1:
		if (role == Qt::DisplayRole)
			return user->uniqueID;
		break;
	}

	return QVariant();
}
