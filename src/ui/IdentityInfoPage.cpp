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

#include "IdentityInfoPage.h"
#include "core/UserIdentity.h"
#include "ContactIDWidget.h"
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
    topLayout->addStretch();
    topLayout->addLayout(createButtons());

    mainLayout->addStretch(1);
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

void IdentityInfoPage::createActions()
{
    actAddContact = new QAction(QIcon(QLatin1String(":/icons/user--plus.png")), tr("Add Contact"), this);
    actChangeAvatar = new QAction(QIcon(QLatin1String(":/icons/image--pencil.png")), tr("Change Avatar"), this);
}

QLayout *IdentityInfoPage::createButtons()
{
    QBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);

    QToolButton *btn = new QToolButton;
    btn->setFixedHeight(23);
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btn->setAutoRaise(true);
    btn->setDefaultAction(actAddContact);
    layout->addWidget(btn);

    btn = new QToolButton;
    btn->setFixedHeight(23);
    btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btn->setAutoRaise(true);
    btn->setDefaultAction(actChangeAvatar);
    layout->addWidget(btn);

    layout->addStretch();
    return layout;
}
