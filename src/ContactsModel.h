#ifndef CONTACTSMODEL_H
#define CONTACTSMODEL_H

#include <QAbstractListModel>
#include <QList>

class ContactsModel : public QAbstractListModel
{
Q_OBJECT
public:
    explicit ContactsModel(QObject *parent = 0);

	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

private:
	QList<class ContactUser*> contacts;

	void populate();
};

#endif // CONTACTSMODEL_H
