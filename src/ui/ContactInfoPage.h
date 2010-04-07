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
	~ContactInfoPage();

public slots:
	void saveNotes();

protected:
	virtual void hideEvent(QHideEvent *);

private:
	class QAction *renameAction, *deleteAction;

	QLabel *avatar, *nickname, *infoText;
	QTextEdit *notesEdit;

	void createActions();
	void createAvatar();
	void createNickname();
	void createInfoText();
	class QLayout *createButtons();
	void createNotes(class QBoxLayout *layout);
};

#endif // CONTACTINFOPAGE_H
