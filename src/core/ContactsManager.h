#ifndef CONTACTSMANAGER_H
#define CONTACTSMANAGER_H

#include <QObject>
#include <QList>
#include "ContactUser.h"

class ContactsManager : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(ContactsManager)

public:
    explicit ContactsManager();

	const QList<ContactUser*> &contacts() const { return pContacts; }
	ContactUser *lookupSecret(const QByteArray &secret) const;

	void connectToAll();

private:
	QList<ContactUser*> pContacts;

	void loadFromSettings();
};

extern ContactsManager *contactsManager;

#endif // CONTACTSMANAGER_H
