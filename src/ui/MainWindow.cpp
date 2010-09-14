/* Torsion - http://github.com/special/torsion
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#include "main.h"
#include "MainWindow.h"
#include "ContactsView.h"
#include "ChatWidget.h"
#include "ContactInfoPage.h"
#include "IdentityInfoPage.h"
#include "NotificationWidget.h"
#include "ContactRequestDialog.h"
#include "ContactAddDialog.h"
#include "core/UserIdentity.h"
#include "core/IncomingRequestManager.h"
#include "core/OutgoingContactRequest.h"
#include "core/ContactsManager.h"
#include "tor/TorControlManager.h"
#include "tor/autoconfig/VidaliaConfigManager.h"
#include "ui/torconfig/TorConfigWizard.h"
#include <QToolBar>
#include <QBoxLayout>
#include <QStackedWidget>
#include <QFrame>
#include <QTextDocument>
#include <QAction>
#include <QTimer>
#include <QMessageBox>
#include <QPushButton>

MainWindow *uiMain = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), torNotificationEnabled(false)
{
    Q_ASSERT(!uiMain);
    uiMain = this;

    setWindowTitle(QLatin1String("Torsion"));

    createActions();

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

    /* Chat area */
    createChatArea();

    /* Contacts */
    createContactsView();

    /* Separator line */
    QFrame *line = new QFrame;
    line->setFrameStyle(QFrame::VLine | QFrame::Plain);

    QPalette p = line->palette();
    p.setColor(QPalette::WindowText, p.color(QPalette::Dark));
    line->setPalette(p);

    /* Put those in the layout */
    layout->addWidget(contactsView);
    layout->addWidget(line);
    layout->addWidget(chatArea);

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

    /* Show the tor offline notification 10 seconds after startup, if Tor is not connected by that time.
     * This avoids obnoxiously showing it for the normal case where Tor should connect *very* quickly. */
    QTimer::singleShot(10000, this, SLOT(enableTorNotification()));

    connect(torManager, SIGNAL(statusChanged(int,int)), this, SLOT(updateTorStatus()));
    updateTorStatus();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    actOptions = new QAction(QIcon(QLatin1String(":/icons/gear.png")), tr("Options"), this);
    actOptions->setEnabled(false);
}

void MainWindow::createContactsView()
{
    contactsView = new ContactsView;
    contactsView->setFixedWidth(175);

    connect(contactsView, SIGNAL(activePageChanged(int,QObject*)), SLOT(contactPageChanged(int,QObject*)));
    contactPageChanged(contactsView->activePage(), contactsView->activeObject());
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

void MainWindow::contactPageChanged(int page, QObject *userObject)
{
    QWidget *old = chatArea->currentWidget();
    QWidget *newWidget = 0;

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

    if (old && !qobject_cast<ChatWidget*>(old))
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

    m_notifications.append(widget);
    connect(widget, SIGNAL(destroyed(QObject*)), SLOT(notificationRemoved(QObject*)));

    return widget;
}

void MainWindow::notificationRemoved(QObject *object)
{
    /* static_cast is necessary to get the correct offset pointer; it's safe because we never
     * actually dereference the pointer. */
    m_notifications.removeOne(static_cast<NotificationWidget*>(object));
}

void MainWindow::openAddContactDialog(UserIdentity *identity)
{
    Q_UNUSED(identity);
    ContactAddDialog dialog(this);
    dialog.exec();
}

void MainWindow::openTorConfig()
{
    TorConfigWizard wizard(this);
    wizard.exec();
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
        contactReqNotification = (NotificationWidget*)0;
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

    connect(request->user, SIGNAL(contactDeleted(ContactUser*)), SLOT(clearRequestNotification(ContactUser*)));
}

void MainWindow::updateOutgoingRequest(OutgoingContactRequest *request)
{
    if (!request && !(request = qobject_cast<OutgoingContactRequest*>(sender())))
        return;

    QString message;

    switch (request->status())
    {
    case OutgoingContactRequest::Accepted:
        message = tr("%1 accepted your contact request!");
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

void MainWindow::clearRequestNotification(ContactUser *user)
{
    /* Find the notification for this user */
    QList<NotificationWidget*> widgets = notifications();
    for (QList<NotificationWidget*>::iterator it = widgets.begin(); it != widgets.end(); ++it)
    {
        if (reinterpret_cast<ContactUser*>((*it)->property("user").value<void*>()) == user)
        {
            (*it)->closeNotification();
            return;
        }
    }
}

void MainWindow::updateTorStatus()
{
    /* Offline notification */
    if ((torManager->status() == Tor::TorControlManager::Connected) ||
        (!torNotificationEnabled && torManager->status() != Tor::TorControlManager::Error))
    {
        if (torNotification)
        {
            torNotification.data()->closeNotification();
            torNotification = (NotificationWidget*)0;
        }

        return;
    }

    QString message;

    if (config->value("tor/configMethod").toString() == QLatin1String("vidalia"))
    {
        VidaliaConfigManager vc;
        if (!vc.isVidaliaRunning())
            message = tr("Vidalia is not running. Click to start Vidalia and Tor");
    }

    if (message.isEmpty())
    {
        switch (torManager->status())
        {
        case Tor::TorControlManager::Error:
            message = tr("Unable to connect to Tor. Make sure Tor is running, or click to reconfigure.");
            break;
        default:
            message = tr("Connecting to Tor. If you have trouble, make sure Tor is running.");
            break;
        }
    }

    if (torNotification)
        torNotification.data()->setMessage(message);
    else
        torNotification = showNotification(message, this, SLOT(openTorConfig()));
}

void MainWindow::enableTorNotification()
{
    torNotificationEnabled = true;
    updateTorStatus();
}

void MainWindow::uiRemoveContact(ContactUser *user)
{
    QMessageBox msg(this);
    msg.setWindowTitle(tr("Remove Contact"));
    msg.setText(tr("Are you sure you want to permanently remove <b>%1</b> from your contacts?").arg(Qt::escape(user->nickname())));
    msg.setIcon(QMessageBox::Question);
    QAbstractButton *deleteBtn = msg.addButton(tr("Remove"), QMessageBox::DestructiveRole);
    msg.addButton(QMessageBox::Cancel);
    msg.setDefaultButton(QMessageBox::Cancel);

    msg.exec();
    if (msg.clickedButton() != deleteBtn)
        return;

    user->deleteContact();
}
