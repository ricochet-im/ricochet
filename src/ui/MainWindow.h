#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

enum ContactPage
{
	ChatPage,
	InfoPage
};

class ContactUser;

class MainWindow : public QMainWindow
{
    Q_OBJECT

	friend class ChatWidget;

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
	virtual void closeEvent(QCloseEvent *);

private slots:
	void contactPageChanged(ContactUser *user, ContactPage page);

private:
	class QStackedWidget *chatArea;

	void createChatArea();
	void addChatWidget(ChatWidget *widget);
};

extern MainWindow *uiMain;

#endif // MAINWINDOW_H
