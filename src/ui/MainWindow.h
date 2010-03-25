#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
	virtual void closeEvent(QCloseEvent *);

private:
	class QStackedWidget *chatArea;

	void createToolbar();
	class QTreeView *createContacts();
	void createChatArea();
};

#endif // MAINWINDOW_H
