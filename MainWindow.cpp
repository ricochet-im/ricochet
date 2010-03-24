#include "MainWindow.h"
#include "ChatWidget.h"
#include <QToolBar>
#include <QBoxLayout>
#include <QTreeView>
#include <QTabWidget>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	createToolbar();

	QWidget *center = new QWidget;
	setCentralWidget(center);

	QBoxLayout *layout = new QHBoxLayout(center);
	layout->setMargin(0);
	layout->setSpacing(0);

	QTreeView *contactsView = createContacts();
	layout->addWidget(contactsView);

	/* Separator line */
	QFrame *line = new QFrame;
	line->setFrameStyle(QFrame::VLine | QFrame::Sunken);
	layout->addWidget(line);

	QTabWidget *chatArea = createChatArea();
	layout->addWidget(chatArea);

	chatArea->addTab(new ChatWidget, "Test");
	chatArea->addTab(new ChatWidget, "Abit");
}

MainWindow::~MainWindow()
{

}

void MainWindow::createToolbar()
{
	QToolBar *toolbar = addToolBar(tr("Main"));
	toolbar->setAllowedAreas(Qt::TopToolBarArea);
	toolbar->setFloatable(false);
	toolbar->setMovable(false);
	toolbar->addAction("Action");
}

QTreeView *MainWindow::createContacts()
{
	QTreeView *contactsView = new QTreeView;
	contactsView->setRootIsDecorated(false);
	contactsView->setHeaderHidden(true);
	contactsView->setFixedWidth(170);
	contactsView->setFrameStyle(QFrame::NoFrame);

	return contactsView;
}

QTabWidget *MainWindow::createChatArea()
{
	QTabWidget *chatArea = new QTabWidget;
	chatArea->setDocumentMode(true);

	return chatArea;
}
