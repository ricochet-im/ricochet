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

#include "IdentityInfoPage.h"
#include "ui/MainWindow.h"
#include "core/UserIdentity.h"
#include "ContactIDWidget.h"
#include "EditableLabel.h"
#include "core/NicknameValidator.h"
#include "utils/PaintUtil.h"
#include <QBoxLayout>
#include <QLabel>
#include <QAction>
#include <QToolButton>

IdentityInfoPage::IdentityInfoPage(UserIdentity *id, QWidget *parent)
    : QWidget(parent), identity(id)
{
    createActions();

    QBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(0);

    QBoxLayout *topLayout = new QHBoxLayout;
    mainLayout->addLayout(topLayout);

    topLayout->addWidget(createAvatar());
    topLayout->addLayout(createInfo());

    mainLayout->addStretch(1);
    mainLayout->addLayout(createButtons());

    connect(identity, SIGNAL(statusChanged()), SLOT(statusChanged()));
}

QWidget *IdentityInfoPage::createAvatar()
{
    m_avatar = new QLabel;
    m_avatar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QPixmap avatar = identity->avatar(FullAvatar);
    if (!avatar.isNull())
        m_avatar->setPixmap(shadowedAvatar(avatar));
    else
        m_avatar->setPixmap(QPixmap(QLatin1String(":/graphics/avatar-placeholder.png")));

    return m_avatar;
}

QLayout *IdentityInfoPage::createInfo()
{
    QGridLayout *layout = new QGridLayout;
    layout->setMargin(0);
    layout->setVerticalSpacing(0);
    layout->setHorizontalSpacing(5);

    int row = 0;

    /* Nickname */
    m_nickname = new EditableLabel;
    m_nickname->setText(identity->nickname());
    m_nickname->addAction(actRename);
    m_nickname->setContextMenuPolicy(Qt::ActionsContextMenu);
    NicknameValidator *validator = new NicknameValidator(m_nickname);
    validator->setWidget(m_nickname);
    m_nickname->setValidator(validator);

    QFont font = m_nickname->font();
    font.setPixelSize(14);
    m_nickname->setFont(font);

    QPalette p = m_nickname->palette();
    p.setColor(QPalette::WindowText, QColor(0x00, 0x66, 0xaa));
    p.setColor(QPalette::Text, QColor(0x00, 0x66, 0xaa));
    m_nickname->setPalette(p);

    connect(actRename, SIGNAL(triggered()), m_nickname, SLOT(startEditing()));
    connect(actRename, SIGNAL(triggered()), m_nickname, SLOT(setFocus()));
    connect(actRename, SIGNAL(triggered()), m_nickname, SLOT(selectAll()));
    connect(m_nickname, SIGNAL(editingFinished()), SLOT(setNickname()));

    layout->addWidget(m_nickname, row++, 0, 1, 2);

    /* ID */
    QLabel *label = new QLabel;
    label->setPixmap(QPixmap(QLatin1String(":/icons/vcard.png")));
    label->setToolTip(tr("Double click to copy your contact ID. Share this to let people add you as a contact."));
    layout->addWidget(label, row, 0);

    m_contactId = new ContactIDWidget;
    m_contactId->setText(identity->contactID());
    m_contactId->setToolTip(label->toolTip());
    m_contactId->setFrame(false);
    m_contactId->setTextMargins(-2, 0, 0, 0);

    QPalette idPalette = m_contactId->palette();
    idPalette.setBrush(QPalette::Base, idPalette.window());
    idPalette.setBrush(QPalette::Text, idPalette.windowText());
    m_contactId->setPalette(idPalette);

    layout->addWidget(m_contactId, row++, 1);
    layout->setRowStretch(row++, 1);
    return layout;
}

void IdentityInfoPage::createActions()
{
    actAddContact = new QAction(QIcon(QLatin1String(":/icons/user--plus.png")), tr("Add Contact"), this);
    connect(actAddContact, SIGNAL(triggered()), SLOT(openAddContactDialog()));
    actChangeAvatar = new QAction(QIcon(QLatin1String(":/icons/image--pencil.png")), tr("Change Avatar"), this);
    actChangeAvatar->setEnabled(false);
    actRename = new QAction(QIcon(QLatin1String(":/icons/user--pencil.png")), tr("Change Nickname"), this);
    actRename->setIconVisibleInMenu(false);
}

QLayout *IdentityInfoPage::createButtons()
{
    QGridLayout *layout = new QGridLayout;
    layout->setMargin(0);
    layout->setHorizontalSpacing(0);

    int row = 0, column = 0;

    layout->setColumnStretch(column++, 1);

    QAction *actions[] = { actAddContact, actChangeAvatar, actRename };
    for (int i = 0; i < 3; ++i)
    {
        QToolButton *btn = new QToolButton;
        btn->setFixedHeight(23);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setAutoRaise(true);
        btn->setDefaultAction(actions[i]);
        layout->addWidget(btn, row++, column);
        if (row == 1)
            row = 0, column++;
    }

    layout->setColumnStretch(column++, 1);
    return layout;
}

void IdentityInfoPage::statusChanged()
{
    if (m_contactId->text().isEmpty())
        m_contactId->setText(identity->contactID());
}

void IdentityInfoPage::setNickname()
{
    if (m_nickname->hasAcceptableInput())
        identity->setNickname(m_nickname->text());
}

void IdentityInfoPage::openAddContactDialog()
{
    uiMain->openAddContactDialog(identity);
}
