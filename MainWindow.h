#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
	void createToolbar();
	class QTreeView *createContacts();
	class QTabWidget *createChatArea();
};

#endif // MAINWINDOW_H
