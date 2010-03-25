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
