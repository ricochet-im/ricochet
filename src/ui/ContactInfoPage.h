#ifndef CONTACTINFOPAGE_H
#define CONTACTINFOPAGE_H

#include <QWidget>

class ContactUser;
class QLabel;
class QTextEdit;

class ContactInfoPage : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(ContactInfoPage)

public:
	ContactUser * const user;

	explicit ContactInfoPage(ContactUser *user, QWidget *parent = 0);

private:
	QLabel *avatar, *nickname;
	QTextEdit *notesEdit;

	void createAvatar();
	void createNickname();
	void createNotes();
};

#endif // CONTACTINFOPAGE_H
