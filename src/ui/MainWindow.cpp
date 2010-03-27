#include "MainWindow.h"
#include "ContactsModel.h"
#include "ContactItemDelegate.h"
#include "ChatWidget.h"
#include "ContactInfoPage.h"
#include "core/ContactsManager.h"
#include <QToolBar>
#include <QBoxLayout>
#include <QTreeView>
#include <QStackedWidget>
#include <QFrame>
#include <QHeaderView>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	setWindowTitle(QString("TorIM"));

	/* Saved geometry */
	QSettings settings;

	resize(settings.value("ui/main/windowSize", QSize(730, 400)).toSize());
	QPoint pos = settings.value("ui/main/windowPosition").toPoint();
	if (!pos.isNull())
		move(pos);

	/* Toolbar */
	createToolbar();

	/* Center widget */
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

	//chatArea->addWidget(new ChatWidget);
	chatArea->addWidget(new ContactInfoPage(contactsManager->contacts()[0]));
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

void MainWindow::closeEvent(QCloseEvent *ev)
{
	QSettings settings;
	settings.setValue("ui/main/windowSize", size());
	settings.setValue("ui/main/windowPosition", pos());

	QMainWindow::closeEvent(ev);
}
