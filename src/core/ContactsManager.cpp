#include "main.h"
#include "ContactsManager.h"
#include <QStringList>

ContactsManager *contactsManager = 0;

ContactsManager::ContactsManager()
{
	loadFromSettings();
}

void ContactsManager::loadFromSettings()
{
	config->beginGroup(QString("contacts"));
	QStringList sections = config->childGroups();
	config->endGroup();

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
		if ((*it)->readSetting("localSecret") == secret)
			return *it;
	}

	return 0;
}
