/* Torsion - http://torsionim.org/
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
    font.setPointSize(11);
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

    QAction *actions[] = { actAddContact, actChangeAvatar, actRename, uiMain->actOptions };
    for (int i = 0; i < 4; ++i)
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
