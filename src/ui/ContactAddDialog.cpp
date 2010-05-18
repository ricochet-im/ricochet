/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, Robin Burchell <robin.burchell@collabora.co.uk>
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

#include "ui/ContactAddDialog.h"
#include "ui/FancyTextEdit.h"
#include "core/ContactsManager.h"
#include "core/OutgoingRequestManager.h"
#include "core/ContactIDValidator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QClipboard>
#include <QApplication>
#include <QDialogButtonBox>
#include <QMessageBox>

ContactAddDialog::ContactAddDialog(QWidget *parent) :
    QDialog(parent),
    m_nickname(new QLineEdit),
    m_id(new QLineEdit),
    m_message(new FancyTextEdit)
{
    setModal(true);
    setWindowTitle(tr("Add Contact"));
    setFixedSize(390, 240);

    m_message->setPlaceholderText(tr("Enter something about yourself here!"));

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(checkClipboardForId()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(createUI());

    // populate UI if there is an id on the clipboard already
    checkClipboardForId();
}

QWidget *ContactAddDialog::createUI() const
{
    QWidget *introPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(introPage);
    layout->setMargin(0);

    // ID
    m_id->setValidator(new ContactIDValidator(m_id));

    // top form
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Nickname:"), m_nickname);
    formLayout->addRow(tr("ID:"), m_id);
    layout->addLayout(formLayout);

    // message
    layout->addWidget(m_message);
    layout->addStretch(1);

    // ok/cancel
    QDialogButtonBox *qdbb = new QDialogButtonBox(Qt::Horizontal);
    qdbb->addButton(QDialogButtonBox::Ok);
    qdbb->addButton(QDialogButtonBox::Cancel);
    connect(qdbb->button(QDialogButtonBox::Ok), SIGNAL(clicked()), SLOT(processFriendAdd()));
    connect(qdbb->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(close()));

    layout->addWidget(qdbb);

    return introPage;
}

void ContactAddDialog::processFriendAdd()
{
    if (!m_id->hasAcceptableInput() || !m_nickname->hasAcceptableInput())
        return;

    QString hostname = ContactIDValidator::hostnameFromID(m_id->text());
    Q_ASSERT(!hostname.isNull());

    if (contactsManager->lookupHostname(hostname))
        return;

    ContactUser *user = contactsManager->addContact(m_nickname->text());
    if (!user)
    {
        QMessageBox::critical(this, tr("Error"), tr("An error occurred while trying to add"
                                                    "the contact. Please try again."));
        return;
    }

    user->setHostname(hostname);

    /* TODO add to OutgoingRequestManager */
    qFatal("Not implemented");
}

void ContactAddDialog::checkClipboardForId()
{
    const QClipboard *clipboard = QApplication::clipboard();

    QRegExp r = QRegExp("(?:^([a-z2-7]{16})$|([a-z2-7]{16})@TorIM)", Qt::CaseInsensitive);
    if (r.indexIn(clipboard->text()) == -1)
        return;

    QStringList sl = r.capturedTexts();
    if (sl.at(0).indexOf(QLatin1String("@TorIM"), 0, Qt::CaseInsensitive) == -1)
        m_id->setText(sl.at(0) + QLatin1String("@TorIM"));
    else
        m_id->setText(sl.at(0));
}
