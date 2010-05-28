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

#include "main.h"
#include "MainWindow.h"
#include "ContactsView.h"
#include "HomeContactWidget.h"
#include "ChatWidget.h"
#include "ContactInfoPage.h"
#include "HomeScreen.h"
#include "NotificationWidget.h"
#include "ContactRequestDialog.h"
#include "core/IncomingRequestManager.h"
#include "core/ContactsManager.h"
#include <QToolBar>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QFrame>

MainWindow *uiMain = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    Q_ASSERT(!uiMain);
    uiMain = this;

    setWindowTitle(QLatin1String("TorIM"));

    /* Saved geometry */
    resize(config->value("ui/main/windowSize", QSize(730, 400)).toSize());
    QPoint pos = config->value("ui/main/windowPosition").toPoint();
    if (!pos.isNull())
        move(pos);

    /* Center widget */
    QWidget *center = new QWidget;
    setCentralWidget(center);

    QBoxLayout *topLayout = new QVBoxLayout(center);
    topLayout->setMargin(0);
    topLayout->setSpacing(0);

    QBoxLayout *layout = new QHBoxLayout;
    topLayout->addLayout(layout);

    /* Contacts */
    QBoxLayout *contactsLayout = new QVBoxLayout;
    layout->addLayout(contactsLayout);

    createContactsView();
    contactsLayout->addWidget(contactsView);

    /* Home contact */
    createHomeContact();
    contactsLayout->addWidget(homeContact);

    /* Separator line */
    QFrame *line = new QFrame;
    line->setFrameStyle(QFrame::VLine | QFrame::Plain);

    QPalette p = line->palette();
    p.setColor(QPalette::WindowText, p.color(QPalette::Dark));
    line->setPalette(p);

    layout->addWidget(line);

    /* Chat area */
    createChatArea();
    layout->addWidget(chatArea);

    homeScreen = new HomeScreen;
    chatArea->addWidget(homeScreen);

    showHomeScreen();

    /* Other things */
    connect(contactsManager->incomingRequests, SIGNAL(requestAdded(IncomingContactRequest*)), SLOT(updateContactRequests()));
    connect(contactsManager->incomingRequests, SIGNAL(requestRemoved(IncomingContactRequest*)), SLOT(updateContactRequests()));

    updateContactRequests();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createContactsView()
{
    contactsView = new ContactsView;
    contactsView->setFixedWidth(175);

    connect(contactsView, SIGNAL(activePageChanged(ContactUser*,ContactPage)), this,
            SLOT(contactPageChanged(ContactUser*,ContactPage)));
}

void MainWindow::createHomeContact()
{
    homeContact = new HomeContactWidget;
    connect(homeContact, SIGNAL(selected()), this, SLOT(showHomeScreen()));
}

void MainWindow::createChatArea()
{
    chatArea = new QStackedWidget;
    chatArea->setContentsMargins(4, 6, 4, 6);
}

void MainWindow::closeEvent(QCloseEvent *ev)
{
    config->setValue("ui/main/windowSize", size());
    config->setValue("ui/main/windowPosition", pos());

    QMainWindow::closeEvent(ev);
}

void MainWindow::addChatWidget(ChatWidget *widget)
{
    chatArea->addWidget(widget);
}

void MainWindow::showHomeScreen()
{
    if (!homeContact->isSelected())
    {
        homeContact->setSelected(true);
        return;
    }

    contactsView->selectionModel()->clearSelection();
    contactsView->setCurrentIndex(QModelIndex());

    chatArea->setCurrentWidget(homeScreen);
}

void MainWindow::contactPageChanged(ContactUser *user, ContactPage page)
{
    QWidget *old = chatArea->currentWidget();
    QWidget *newWidget = 0;

    homeContact->clearSelected();

    switch (page)
    {
    case ChatPage:
        newWidget = ChatWidget::widgetForUser(user);
        break;
    case InfoPage:
        newWidget = new ContactInfoPage(user);
        break;
    default:
        Q_ASSERT_X(false, "contactPageChanged", "Called for unimplemented page type");
    }

    if (old == newWidget)
        return;

    if (old && !qobject_cast<ChatWidget*>(old) && old != homeScreen)
        old->deleteLater();

    if (newWidget)
        chatArea->setCurrentIndex(chatArea->addWidget(newWidget));
}

NotificationWidget *MainWindow::showNotification(const QString &message, QObject *receiver, const char *slot)
{
    NotificationWidget *widget = new NotificationWidget(message);
    if (receiver && slot)
        connect(widget, SIGNAL(clicked()), receiver, slot);

    Q_ASSERT(centralWidget() && qobject_cast<QBoxLayout*>(centralWidget()->layout()));
    QBoxLayout *mainLayout = static_cast<QBoxLayout*>(centralWidget()->layout());
    int mainIndex = mainLayout->count() - 1;

    mainLayout->insertWidget(mainIndex, widget);
    widget->showAnimated();

    return widget;
}

void MainWindow::updateContactRequests()
{
    int numRequests = contactsManager->incomingRequests->requests().size();

    if (numRequests && !contactReqNotification)
    {
        contactReqNotification = showNotification(tr("Someone wants to contact you - click here for more information"),
                                                  this, SLOT(showContactRequest()));
    }
    else if (!numRequests && contactReqNotification)
    {
        contactReqNotification.data()->closeNotification();
        contactReqNotification.clear();
    }
}

void MainWindow::showContactRequest()
{
    QList<IncomingContactRequest*> requests = contactsManager->incomingRequests->requests();

    for (QList<IncomingContactRequest*>::Iterator it = requests.begin(); it != requests.end(); ++it)
    {
        ContactRequestDialog *dialog = new ContactRequestDialog(*it, this);

        /* Allow the user a way out of a loop of requests by cancelling */
        if (dialog->exec() == ContactRequestDialog::Cancelled)
            break;
    }

    updateContactRequests();
}
