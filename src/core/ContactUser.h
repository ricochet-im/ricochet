#ifndef CONTACTUSER_H
#define CONTACTUSER_H

#include <QObject>

class ContactUser : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(ContactUser)

	Q_PROPERTY(QString nickname READ nickname WRITE setNickname STORED true)

public:
	const QString uniqueID;

    explicit ContactUser(const QString &uniqueID, QObject *parent = 0);

	const QString &nickname() const { return pNickname; }

public slots:
	void setNickname(const QString &nickname);

private:
	QString pNickname;

	void loadSettings();
};

#endif // CONTACTUSER_H
