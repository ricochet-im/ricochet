#include "MainWindow.h"
#include "ContactsView.h"
#include "ChatWidget.h"
#include "ContactInfoPage.h"
#include "core/ContactsManager.h"
#include <QToolBar>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QFrame>
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
	ContactsView *contactsView = new ContactsView;
	contactsView->setFixedWidth(175);
	layout->addWidget(contactsView);

	connect(contactsView, SIGNAL(activePageChanged(ContactUser*,ContactPage)), this,
			SLOT(contactPageChanged(ContactUser*,ContactPage)));

	/* Separator line */
	QFrame *line = new QFrame;
	line->setFrameStyle(QFrame::VLine | QFrame::Sunken);
	layout->addWidget(line);

	/* Chat area */
	createChatArea();
	layout->addWidget(chatArea);

	contactPageChanged(contactsView->activeContact(), contactsView->activeContactPage());
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

void MainWindow::contactPageChanged(ContactUser *user, ContactPage page)
{
	/* TODO: Keep widgets around when relevant */

	QWidget *old = chatArea->currentWidget();
	QWidget *newWidget = 0;

	switch (page)
	{
	case ChatPage:
		newWidget = new ChatWidget;
		break;
	case InfoPage:
		newWidget = new ContactInfoPage(user);
		break;
	default:
		Q_ASSERT_X(false, "contactPageChanged", "Called for unimplemented page type");
	}

	if (old)
		old->deleteLater();

	if (newWidget)
		chatArea->setCurrentIndex(chatArea->addWidget(newWidget));
}
