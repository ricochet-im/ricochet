#include "ContactsModel.h"

ContactsModel::ContactsModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

int ContactsModel::rowCount(const QModelIndex &parent) const
{
	if (!parent.isValid())
		return 2;
	return 0;
}

int ContactsModel::columnCount(const QModelIndex &parent) const
{
	if (!parent.isValid())
		return 2;
	return 0;
}

QVariant ContactsModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	switch (index.column())
	{
	case 0:
		if (role == Qt::DisplayRole)
			return QString("BestFriend");
		break;
	case 1:
		if (role == Qt::DisplayRole)
			return QString("fi93k293k49023qz");
		break;
	}

	return QVariant();
}
