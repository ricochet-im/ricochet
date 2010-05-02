/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

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
