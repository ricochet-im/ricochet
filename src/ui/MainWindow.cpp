#include "MainWindow.h"
#include "ContactsModel.h"
#include "ContactItemDelegate.h"
#include "ChatWidget.h"
#include <QToolBar>
#include <QBoxLayout>
#include <QTreeView>
#include <QStackedWidget>
#include <QFrame>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	createToolbar();

	QWidget *center = new QWidget;
	setCentralWidget(center);

	QBoxLayout *layout = new QHBoxLayout(center);
	layout->setMargin(0);
	layout->setSpacing(0);

	/* Contacts */
	QTreeView *contactsView = createContacts();
	layout->addWidget(contactsView);

	/* Separator line */
	QFrame *line = new QFrame;
	line->setFrameStyle(QFrame::VLine | QFrame::Sunken);
	layout->addWidget(line);

	/* Chat area */
	createChatArea();
	layout->addWidget(chatArea);

	chatArea->addWidget(new ChatWidget);
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
	toolbar->addAction(QIcon(":/icons/user-add"), tr("Add Contact"));
}

QTreeView *MainWindow::createContacts()
{
	QTreeView *contactsView = new QTreeView;
	contactsView->setRootIsDecorated(false);
	contactsView->setHeaderHidden(true);
	contactsView->setFixedWidth(175);
	contactsView->setFrameStyle(QFrame::NoFrame);

	contactsView->setModel(new ContactsModel(contactsView));
	contactsView->setItemDelegate(new ContactItemDelegate(contactsView));

	QHeaderView *header = contactsView->header();
	for (int i = 1; i < header->count(); ++i)
		header->hideSection(i);

	header->setResizeMode(0, QHeaderView::Stretch);

	return contactsView;
}

void MainWindow::createChatArea()
{
	chatArea = new QStackedWidget;
	chatArea->setContentsMargins(4, 6, 4, 6);
}
