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

#include "main.h"
#include "ContactRequestDialog.h"
#include "core/ContactsManager.h"
#include "core/IncomingRequestManager.h"
#include "core/NicknameValidator.h"
#include "utils/DateUtil.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QTextEdit>
#include <QDateTime>
#include <QToolTip>

ContactRequestDialog::ContactRequestDialog(IncomingContactRequest *r, QWidget *parent)
    : QDialog(parent), request(r)
{
    setWindowTitle(tr("Contact Request"));
    setModal(true);
    setFixedSize(450, 200);

    QBoxLayout *mainLayout = new QVBoxLayout(this);

    /* Introduction */
    QLabel *intro = new QLabel(tr("Someone has requested to add you as a contact"));
    intro->setStyleSheet(QLatin1String("font-weight:bold;"));
    mainLayout->addWidget(intro);

    QTextEdit *message = new QTextEdit;
    message->setReadOnly(true);
    QFont f;
    f.setPixelSize(12);
    f.setStyleHint(QFont::SansSerif);
    message->setFont(f);

    message->setText(QString::fromLatin1(
            "<table><tr><td style='color:#808080;'>%1</td><td width='100%' style='font-family:Consolas,\"Courier New\";"
            "font-weight:bold;'>%2</td></tr><tr><td style='color:#808080;padding-right:9px;'>%3</td><td>%4</td></tr><tr>"
            "<td colspan=2><br>%5</td></tr></table>")
            .arg(tr("ID:"))
            .arg(QString::fromLatin1(request->hostname) + QLatin1String("@Torsion"))
            .arg(tr("Date:"))
            .arg(tr("%1 (%2)").arg(timeDifferenceString(request->requestDate()),
                                   request->requestDate().toString(Qt::SystemLocaleShortDate)))
            .arg(Qt::escape(request->message()).replace(QLatin1Char('\n'), QLatin1String("<br>")))
    );

    mainLayout->addWidget(message);

    QBoxLayout *bLayout = new QHBoxLayout;
    mainLayout->addLayout(bLayout);

    /* Nickname */
    QLabel *label = new QLabel(tr("Nickname:"));
    bLayout->addWidget(label);

    m_nickname = new QLineEdit;
#if QT_VERSION >= 0x040700
    m_nickname->setPlaceholderText(tr("Enter a nickname for this contact"));
#endif
    NicknameValidator *nickValidator = new NicknameValidator(m_nickname);
    nickValidator->setWidget(m_nickname);
    nickValidator->setValidateUnique(r->manager->contacts->identity);
    m_nickname->setValidator(nickValidator);
    m_nickname->setFixedWidth(200);
    bLayout->addWidget(m_nickname, 1, Qt::AlignLeft | Qt::AlignVCenter);

    /* Buttons */
    QDialogButtonBox *btns = new QDialogButtonBox(Qt::Horizontal);
    btns->addButton(tr("Accept"), QDialogButtonBox::YesRole);
    btns->addButton(tr("Reject"), QDialogButtonBox::NoRole);
    connect(btns, SIGNAL(accepted()), this, SLOT(accept()));
    connect(btns, SIGNAL(rejected()), this, SLOT(rejectRequest()));

    bLayout->addWidget(btns);

    /* Other */
    connect(&r->manager->contacts->incomingRequests, SIGNAL(requestRemoved(IncomingContactRequest*)),
            SLOT(requestRemoved(IncomingContactRequest*)));
}

void ContactRequestDialog::accept()
{
    if (!m_nickname->hasAcceptableInput())
    {
        m_nickname->setFocus();
        QToolTip::showText(m_nickname->mapToGlobal(QPoint(0,0)),
                           tr("You must enter a valid nickname for this contact"), m_nickname);
        return;
    }

    /* Disconnect from the requestRemoved signal to avoid hitting it here */
    request->manager->contacts->incomingRequests.disconnect(this, SLOT(requestRemoved(IncomingContactRequest*)));

    /* Accept request */
    request->setNickname(m_nickname->text());
    request->accept();

    /* Close dialog */
    QDialog::accept();
}

void ContactRequestDialog::rejectRequest()
{
    /* Disconnect from the requestRemoved signal to avoid hitting it here */
    request->manager->contacts->incomingRequests.disconnect(this, SLOT(requestRemoved(IncomingContactRequest*)));

    request->reject();
    this->done(QDialog::Rejected);
}

void ContactRequestDialog::reject()
{
    this->done(ContactRequestDialog::Cancelled);
}

void ContactRequestDialog::requestRemoved(IncomingContactRequest *r)
{
    if (r != request)
        return;

    /* Request is gone, there is no way for this dialog to stay open (the request will soon be an invalid pointer). */
    reject();
}
