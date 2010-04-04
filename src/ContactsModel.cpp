#include "ContactsModel.h"
#include "core/ContactsManager.h"
#include <QImage>
#include <QColor>

static inline bool contactSort(const ContactUser *u1, const ContactUser *u2)
{
	int p1 = u1->readSetting(QString("listPosition"), -1).toInt();
	int p2 = u2->readSetting(QString("listPosition"), -1).toInt();
	if (p2 < 0)
		return true;
	else if (p1 < 0)
		return false;
	else
		return (p1 < p2);
}

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
	qSort(contacts.begin(), contacts.end(), contactSort);

	for (QList<ContactUser*>::Iterator it = contacts.begin(); it != contacts.end(); ++it)
	{
		connect(*it, SIGNAL(connected()), this, SLOT(updateUser()));
		connect(*it, SIGNAL(disconnected()), this, SLOT(updateUser()));
		connect(*it, SIGNAL(statusLineChanged()), this, SLOT(updateUser()));
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

void ContactsModel::moveRow(int from, int to)
{
	if (from < 0 || from >= contacts.size() || to < 0 || to >= contacts.size() || from == to)
		return;

	emit layoutAboutToBeChanged();
	/* A shortcut is taken here by not changing persistent indexes for everything, but it isn't necessary
	 * as used right now. */
	for (int i = 0; i < columnCount(); ++i)
	{
		changePersistentIndex(index(from, i, QModelIndex()), index(to, i, QModelIndex()));
		changePersistentIndex(index(to, i, QModelIndex()), index(from, i, QModelIndex()));
	}

	contacts.move(from, to);

	/* Update the stored positions */
	for (int i = 0; i < contacts.size(); ++i)
	{
		if (contacts[i]->readSetting(QString("listPosition"), -1).toInt() != i)
			contacts[i]->writeSetting(QString("listPosition"), i);
	}

	emit layoutChanged();
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
		return 3;
}

Qt::DropActions ContactsModel::supportedDropActions() const
{
	return Qt::MoveAction;
}

Qt::ItemFlags ContactsModel::flags(const QModelIndex &index) const
{
	if (index.isValid())
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	else
		return 0;//Qt::ItemIsDropEnabled;
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
	case 2:
		if (role == Qt::DisplayRole)
			return user->statusLine();
		break;
	}

	return QVariant();
}
