/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ui/ContactAddDialog.h"
#include "ui/FancyTextEdit.h"
#include "core/ContactsManager.h"
#include "core/IdentityManager.h"
#include "core/OutgoingContactRequest.h"
#include "core/NicknameValidator.h"
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
    m_message(new FancyTextEdit),
    m_buttonBox(new QDialogButtonBox(Qt::Horizontal)),
    m_identity(0)
{
    setModal(true);
    setWindowTitle(tr("Add Contact"));
    setFixedSize(390, 240);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(checkClipboardForId()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(createUI());

    // populate UI if there is an id on the clipboard already
    checkClipboardForId();
    updateAcceptableInput();
}

QWidget *ContactAddDialog::createUI()
{
    QWidget *introPage = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(introPage);
    layout->setMargin(0);

    // Nickname
    m_nickname->setWhatsThis(tr("Choose a nickname for this contact. You can use any name."));
    m_nickname->setValidator(new NicknameValidator(m_nickname));
    connect(m_nickname, SIGNAL(textChanged(QString)), SLOT(updateAcceptableInput()));

    // ID
    m_id->setWhatsThis(tr("Enter the ID of your contact, e.g. w3rf2xcq1b88lbda@Torsion"));
    m_id->setValidator(new ContactIDValidator(m_id));
    connect(m_id, SIGNAL(textChanged(QString)), SLOT(updateAcceptableInput()));

    // top form
    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("ID:"), m_id);
    formLayout->addRow(tr("Nickname:"), m_nickname);
    layout->addLayout(formLayout);

    // message
    m_message->setWhatsThis(tr("This message is sent along with your request. Write something about yourself."));
    m_message->setTabChangesFocus(true);
    m_message->setPlaceholderText(tr("Enter a message for your request.\nTell your contact who you are, or why "
                                     "they should accept."));

    connect(m_message, SIGNAL(textChanged()), this, SLOT(updateAcceptableInput()));

    layout->addWidget(m_message);
    layout->addStretch(1);

    // ok/cancel
    m_buttonBox->addButton(QDialogButtonBox::Ok);
    m_buttonBox->addButton(QDialogButtonBox::Cancel);
    connect(m_buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    layout->addWidget(m_buttonBox);

    return introPage;
}

void ContactAddDialog::setIdentity(UserIdentity *identity)
{
    m_identity = identity;
    updateAcceptableInput();
}

void ContactAddDialog::accept()
{
    if (!hasAcceptableInput())
        return;

    QString hostname = ContactIDValidator::hostnameFromID(m_id->text());
    Q_ASSERT(!hostname.isNull());

    if (identityManager->lookupHostname(hostname))
    {
        QMessageBox::critical(this, tr("Error"), tr("You can't add yourself as a contact."));
        return;
    }

    ContactUser *user;

    if ((user = m_identity->contacts.lookupHostname(hostname)))
    {
        QMessageBox::critical(this, tr("Error"), tr("You already have <b>%1</b> as a contact.")
                              .arg(Qt::escape(user->nickname())));
        return;
    }

    if ((user = m_identity->contacts.lookupNickname(m_nickname->text())))
    {
        QMessageBox::critical(this, tr("Error"), tr("You already have another contact named "
                                                    "<b>%1</b>. Please choose another nickname.")
                              .arg(Qt::escape(user->nickname())));
        m_nickname->setFocus(Qt::OtherFocusReason);
        m_nickname->selectAll();
        return;
    }

    user = m_identity->contacts.addContact(m_nickname->text());
    if (!user)
    {
        QMessageBox::critical(this, tr("Error"), tr("An error occurred while trying to add"
                                                    "the contact. Please try again."));
        return;
    }

    user->setHostname(hostname);
    OutgoingContactRequest::createNewRequest(user, QString(), m_message->document()->toPlainText());

    QDialog::accept();
}

bool ContactAddDialog::hasAcceptableInput() const
{
    return m_nickname->hasAcceptableInput() && m_id->hasAcceptableInput()
            && !m_message->isTextEmpty() && m_identity;
}

void ContactAddDialog::updateAcceptableInput()
{
    m_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasAcceptableInput());
}

void ContactAddDialog::checkClipboardForId()
{
    const QClipboard *clipboard = QApplication::clipboard();

    QRegExp r = QRegExp(QLatin1String("(?:^([a-z2-7]{16})$|([a-z2-7]{16})@Torsion)"), Qt::CaseInsensitive);
    if (r.indexIn(clipboard->text()) == -1)
        return;

    QStringList sl = r.capturedTexts();
    if (sl.at(0).indexOf(QLatin1String("@Torsion"), 0, Qt::CaseInsensitive) == -1)
        m_id->setText(sl.at(0) + QLatin1String("@Torsion"));
    else
        m_id->setText(sl.at(0));
}
