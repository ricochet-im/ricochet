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
	enum AvatarSize
	{
		FullAvatar, /* 160x160 */
		TinyAvatar /* 35x35 */
	};

	const QString uniqueID;

	explicit ContactUser(const QString &uniqueID, QObject *parent = 0);

	const QString &nickname() const { return pNickname; }

	QPixmap avatar(AvatarSize size);

public slots:
	void setNickname(const QString &nickname);
	void setAvatar(QImage image);

private:
	QString pNickname;
	QPixmapCache::Key cachedAvatar[2];

	void loadSettings();
};

#endif // CONTACTUSER_H
