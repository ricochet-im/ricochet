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
	contacts = contactsManager->contacts();
	endResetModel();
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
