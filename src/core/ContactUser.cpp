#include "ContactUser.h"
#include <QSettings>
#include <QPixmapCache>
#include <QtDebug>

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

QImage ContactUser::avatar(const QSize &size) const
{
	QSettings settings;
	QImage re = settings.value(QString("contacts/%1/avatar").arg(uniqueID)).value<QImage>();
	if (re.isNull())
		return re;

	if (size.isValid() && (re.width() > size.width() || re.height() > size.height()))
		re = re.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

	return re;
}

QPixmap ContactUser::avatarCache(const QSize &size)
{
	QHash<QSize,QPixmapCache::Key>::Iterator it = cachedAvatars.find(size);
	if (it != cachedAvatars.end())
	{
		QPixmap re;
		if (QPixmapCache::find(*it, &re))
			return re;
	}

	qDebug() << "Generating cached avatar for" << uniqueID << "at" << size;

	QPixmap re = QPixmap::fromImage(avatar(size));
	cachedAvatars.insert(size, QPixmapCache::insert(re));
	return re;
}

void ContactUser::setAvatar(const QImage &image)
{
	QSettings settings;

	QString key = QString("contacts/%1/avatar").arg(uniqueID);
	if (image.isNull())
		settings.remove(key);
	else
		settings.setValue(key, image);

	clearCachedAvatars();
}

void ContactUser::loadSettings()
{
	QSettings settings;
	settings.beginGroup(QString("contacts/").append(uniqueID));

	pNickname = settings.value(QString("nickname"), uniqueID).toString();
}

void ContactUser::clearCachedAvatars()
{
	for (QHash<QSize,QPixmapCache::Key>::Iterator it = cachedAvatars.begin(); it != cachedAvatars.end(); ++it)
		QPixmapCache::remove(*it);
	cachedAvatars.clear();
}
