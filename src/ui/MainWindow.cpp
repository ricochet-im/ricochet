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
#include "IdentityInfoPage.h"
#include "HomeScreen.h"
#include "NotificationWidget.h"
#include "ContactRequestDialog.h"
#include "core/UserIdentity.h"
#include "core/IncomingRequestManager.h"
#include "core/OutgoingContactRequest.h"
#include "core/ContactsManager.h"
#include <QToolBar>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QFrame>
#include <QTextDocument>

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
    connect(contactsManager, SIGNAL(outgoingRequestAdded(OutgoingContactRequest*)), SLOT(outgoingRequestAdded(OutgoingContactRequest*)));

    foreach (ContactUser *user, contactsManager->contacts())
    {
        if (user->isContactRequest())
            outgoingRequestAdded(OutgoingContactRequest::requestForUser(user));
    }

    updateContactRequests();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createContactsView()
{
    contactsView = new ContactsView;
    contactsView->setFixedWidth(175);

    connect(contactsView, SIGNAL(activePageChanged(int,QObject*)), SLOT(contactPageChanged(int,QObject*)));
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

void MainWindow::contactPageChanged(int page, QObject *userObject)
{
    QWidget *old = chatArea->currentWidget();
    QWidget *newWidget = 0;

    homeContact->clearSelected();

    switch (page)
    {
    case ContactsView::ContactChatPage:
        newWidget = ChatWidget::widgetForUser(static_cast<ContactUser*>(userObject));
        break;
    case ContactsView::ContactInfoPage:
        newWidget = new ContactInfoPage(static_cast<ContactUser*>(userObject));
        break;
    case ContactsView::IdentityInfoPage:
        newWidget = new IdentityInfoPage(static_cast<UserIdentity*>(userObject));
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
    /* This cannot iterate a list because it is technically possible for that list to change during the loop */
    for (;;)
    {
        QList<IncomingContactRequest*> requests = contactsManager->incomingRequests->requests();
        if (requests.isEmpty())
            break;

        ContactRequestDialog *dialog = new ContactRequestDialog(requests.first(), this);

        /* Allow the user a way out of a loop of requests by cancelling */
        if (dialog->exec() == ContactRequestDialog::Cancelled)
            break;

        /* Accept or reject would remove the request from the list, so it will not come up again. */
    }

    updateContactRequests();
}

void MainWindow::outgoingRequestAdded(OutgoingContactRequest *request)
{
    connect(request, SIGNAL(statusChanged(int,int)), SLOT(updateOutgoingRequest()));
    updateOutgoingRequest(request);
}

void MainWindow::updateOutgoingRequest(OutgoingContactRequest *request)
{
    if (!request && !(request = qobject_cast<OutgoingContactRequest*>(sender())))
        return;

    QString message;

    switch (request->status())
    {
    case OutgoingContactRequest::Accepted:
        message = tr("%1 accepted your contact request!").arg(Qt::escape(request->user->nickname()));
        break;
    case OutgoingContactRequest::Error:
        message = tr("There was an error with your contact request to %1 - click for more information");
        break;
    case OutgoingContactRequest::Rejected:
        message = tr("%1 rejected your contact request");
        break;
    default:
        break;
    }

    if (message.isEmpty())
        return;

    NotificationWidget *widget = showNotification(message.arg(Qt::escape(request->user->nickname())), this, SLOT(showRequestInfo()));
    widget->setProperty("user", QVariant::fromValue(reinterpret_cast<void*>(request->user)));
    connect(widget, SIGNAL(clicked()), widget, SLOT(closeNotification()));
}

void MainWindow::showRequestInfo()
{
    ContactUser *user = reinterpret_cast<ContactUser*>(sender()->property("user").value<void*>());
    if (!user)
        return;

    contactsView->showContactInfo(user);
}
