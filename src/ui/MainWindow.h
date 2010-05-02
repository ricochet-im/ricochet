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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

enum ContactPage
{
	ChatPage,
	InfoPage
};

class ContactUser;
class ChatWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

	friend class ChatWidget;

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void showHomeScreen();

protected:
	virtual void closeEvent(QCloseEvent *);

private slots:
	void contactPageChanged(ContactUser *user, ContactPage page);

private:
	class ContactsView *contactsView;
	class HomeScreen *homeScreen;
	class HomeContactWidget *homeContact;
	class QStackedWidget *chatArea;

	void createContactsView();
	void createHomeContact();
	void createChatArea();
	void addChatWidget(ChatWidget *widget);
};

extern MainWindow *uiMain;

#endif // MAINWINDOW_H
