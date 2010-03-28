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

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private slots:
	void updateUser(ContactUser *user = 0);

private:
	QList<ContactUser*> contacts;

	void populate();
};

#endif // CONTACTSMODEL_H
