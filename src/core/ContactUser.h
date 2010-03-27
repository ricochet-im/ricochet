#ifndef CONTACTUSER_H
#define CONTACTUSER_H

#include <QObject>
#include <QImage>
#include <QPixmap>
#include <QHash>
#include <QPixmapCache>

class ContactUser : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(ContactUser)

	Q_PROPERTY(QString nickname READ nickname WRITE setNickname STORED true)

public:
	const QString uniqueID;

	explicit ContactUser(const QString &uniqueID, QObject *parent = 0);

	const QString &nickname() const { return pNickname; }

	QImage avatar(const QSize &size = QSize()) const;
	QPixmap avatarCache(const QSize &size);

public slots:
	void setNickname(const QString &nickname);
	void setAvatar(const QImage &image);

private:
	QString pNickname;
	QHash<QSize,QPixmapCache::Key> cachedAvatars;

	void loadSettings();
	void clearCachedAvatars();
};

inline uint qHash(const QSize &size)
{
	return (unsigned(size.width()) << 16) | (unsigned(size.height()) & 0xffff);
}

#endif // CONTACTUSER_H
