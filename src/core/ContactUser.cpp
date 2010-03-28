#include "main.h"
#include "ContactUser.h"
#include <QPixmapCache>
#include <QtDebug>

ContactUser::ContactUser(const QString &id, QObject *parent)
	: QObject(parent), uniqueID(id)
{
	Q_ASSERT(!uniqueID.isEmpty());

	loadSettings();

	QString host = readSetting("hostname").toString();
	quint16 port = (quint16)readSetting("port", 13535).toUInt();
	pConn = new ProtocolManager(host, port, this);

	pConn->setSecret(readSetting("remoteSecret").toByteArray());
}

void ContactUser::loadSettings()
{
	config->beginGroup(QString("contacts/").append(uniqueID));

	pNickname = config->value(QString("nickname"), uniqueID).toString();

	config->endGroup();
}

QVariant ContactUser::readSetting(const QString &key, const QVariant &defaultValue)
{
	return config->value(QString("contacts/%1/%2").arg(uniqueID, key), defaultValue);
}

void ContactUser::writeSetting(const QString &key, const QVariant &value)
{
	config->setValue(QString("contacts/%1/%2").arg(uniqueID, key), value);
}

void ContactUser::setNickname(const QString &nickname)
{
	if (pNickname == nickname)
		return;

	pNickname = nickname;

	config->setValue(QString("contacts/%1/nickname").arg(uniqueID), nickname);
}

QPixmap ContactUser::avatar(AvatarSize size)
{
	QPixmap re;
	if (QPixmapCache::find(cachedAvatar[size], &re))
		return re;

	QString settingsKey = QString("contacts/%1/avatar").arg(uniqueID);
	if (size == TinyAvatar)
		settingsKey.append("-tiny");

	re = QPixmap::fromImage(config->value(settingsKey).value<QImage>());

	cachedAvatar[size] = QPixmapCache::insert(re);
	return re;
}

void ContactUser::setAvatar(QImage image)
{
	if (image.width() > 160 || image.height() > 160)
		image = image.scaled(QSize(160, 160), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	QString key = QString("contacts/%1/avatar").arg(uniqueID);

	if (image.isNull())
	{
		config->remove(key);
		config->remove(key + "-tiny");
	}
	else
	{
		config->setValue(key, image);

		QImage tiny = image.scaled(QSize(35, 35), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		config->setValue(key + "-tiny", tiny);
	}

	for (int i = 0; i < 2; ++i)
		QPixmapCache::remove(cachedAvatar[i]);
}

QString ContactUser::notesText() const
{
	return config->value(QString("contacts/%1/notes").arg(uniqueID)).toString();
}

void ContactUser::setNotesText(const QString &text)
{
	QString key = QString("contacts/%1/notes").arg(uniqueID);

	if (text.isEmpty())
		config->remove(key);
	else
		config->setValue(key, text);
}
