#include "ContactUser.h"
#include <QSettings>

ContactUser::ContactUser(const QString &id, QObject *parent)
	: QObject(parent), uniqueID(id)
{
	Q_ASSERT(!uniqueID.isEmpty());

	loadSettings();
}

void ContactUser::setNickname(const QString &nickname)
{
	if (pNickname == nickname)
		return;

	pNickname = nickname;

	QSettings settings;
	settings.setValue(QString("contacts/%1/nickname").arg(uniqueID), nickname);
}

void ContactUser::loadSettings()
{
	QSettings settings;
	settings.beginGroup(QString("contacts/").append(uniqueID));

	pNickname = settings.value(QString("nickname"), uniqueID).toString();
}
