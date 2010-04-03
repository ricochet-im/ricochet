#include "main.h"
#include "ContactUser.h"
#include "utils/DateUtil.h"
#include <QPixmapCache>
#include <QtDebug>
#include <QBuffer>
#include <QDateTime>

ContactUser::ContactUser(const QString &id, QObject *parent)
	: QObject(parent), uniqueID(id)
{
	Q_ASSERT(!uniqueID.isEmpty());

	loadSettings();

	QString host = readSetting("hostname").toString();
	quint16 port = (quint16)readSetting("port", 13535).toUInt();
	pConn = new ProtocolManager(this, host, port);

	pConn->setSecret(readSetting("remoteSecret").toByteArray());

	connect(pConn, SIGNAL(primaryConnected()), this, SLOT(onConnected()));
	connect(pConn, SIGNAL(primaryDisconnected()), this, SLOT(onDisconnected()));
}

void ContactUser::loadSettings()
{
	config->beginGroup(QString("contacts/").append(uniqueID));

	pNickname = config->value(QString("nickname"), uniqueID).toString();

	config->endGroup();
}

QVariant ContactUser::readSetting(const QString &key, const QVariant &defaultValue) const
{
	return config->value(QString("contacts/%1/%2").arg(uniqueID, key), defaultValue);
}

void ContactUser::writeSetting(const QString &key, const QVariant &value)
{
	config->setValue(QString("contacts/%1/%2").arg(uniqueID, key), value);
}

QString ContactUser::statusLine() const
{
	if (isConnected())
	{
		return tr("Online");
	}
	else
	{
		QDateTime lastConnected = readSetting(QString("lastConnected")).toDateTime();
		if (lastConnected.isNull())
			return tr("Never connected");
		return timeDifferenceString(lastConnected, QDateTime::currentDateTime());
	}
}

void ContactUser::onConnected()
{
	emit connected();

	writeSetting(QString("lastConnected"), QDateTime::currentDateTime());
}

void ContactUser::onDisconnected()
{
	emit disconnected();

	writeSetting(QString("lastConnected"), QDateTime::currentDateTime());
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

	re.loadFromData(config->value(settingsKey).toByteArray());

	cachedAvatar[size] = QPixmapCache::insert(re);
	return re;
}

void ContactUser::setAvatar(QImage image)
{
	if (image.width() > 160 || image.height() > 160)
		image = image.scaled(QSize(160, 160), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	QString key = QString("contacts/%1/avatar").arg(uniqueID);

	if (!image.isNull())
	{
		QBuffer buffer;
		buffer.open(QBuffer::ReadWrite);
		if (image.save(&buffer, "jpeg", 100))
		{
			config->setValue(key, buffer.buffer());

			QImage tiny = image.scaled(QSize(35, 35), Qt::KeepAspectRatio, Qt::SmoothTransformation);
			buffer.close();
			buffer.open(QBuffer::ReadWrite);
			if (tiny.save(&buffer, "jpeg", 100))
				config->setValue(key + "-tiny", buffer.buffer());
			else
				image = QImage();
		}
		else
			image = QImage();
	}

	if (image.isNull())
	{
		config->remove(key);
		config->remove(key + "-tiny");
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
