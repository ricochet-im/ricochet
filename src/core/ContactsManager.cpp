#include "ContactsManager.h"
#include <QSettings>
#include <QStringList>

ContactsManager *contactsManager = 0;

ContactsManager::ContactsManager()
{
	loadFromSettings();
}

void ContactsManager::loadFromSettings()
{
	QSettings settings;

	settings.beginGroup(QString("contacts"));

	QStringList sections = settings.childGroups();
	for (QStringList::Iterator it = sections.begin(); it != sections.end(); ++it)
	{
		ContactUser *user = new ContactUser(*it, this);
		pContacts.append(user);
	}
}

ContactUser *ContactsManager::lookupSecret(const QByteArray &secret) const
{
	Q_ASSERT(secret.size() == 16);

	for (QList<ContactUser*>::ConstIterator it = pContacts.begin(); it != pContacts.end(); ++it)
	{
		if ((*it)->secret() == secret)
			return *it;
	}

	return 0;
}
