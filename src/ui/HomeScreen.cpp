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
#include "HomeScreen.h"
#include "tor/TorControlManager.h"
#include "torconfig/TorConfigWizard.h"
#include "ui/ContactAddDialog.h"
#include <QApplication>
#include <QBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QAction>

HomeScreen::HomeScreen(QWidget *parent)
    : QWidget(parent)
{
    QBoxLayout *layout = new QVBoxLayout(this);
    layout->setMargin(3);

    QBoxLayout *topLayout = new QHBoxLayout;
    layout->addLayout(topLayout);

    createActions();

    createAvatar();

    topLayout->addLayout(createButtons());

    topLayout->addStretch(1);
    topLayout->addWidget(avatar);
    layout->addStretch(1);

    QFrame *line = new QFrame;
    line->setFrameStyle(QFrame::HLine | QFrame::Plain);

    QPalette p = line->palette();
    p.setColor(QPalette::WindowText, p.color(QPalette::Mid));
    line->setPalette(p);

    layout->addWidget(line);

    layout->addWidget(createStatus());
}

void HomeScreen::createAvatar()
{
    avatar = new QLabel;
    avatar->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    avatar->addAction(actChangeAvatar);
    avatar->setContextMenuPolicy(Qt::ActionsContextMenu);

    QImage image = config->value("core/avatar").value<QImage>();
    if (!image.isNull())
        avatar->setPixmap(QPixmap::fromImage(image));
    else
        avatar->setPixmap(QPixmap(QLatin1String(":/graphics/avatar-placeholder.png")));
}

void HomeScreen::createActions()
{
    actAddContact = new QAction(QIcon(QLatin1String(":/icons/user--plus.png")), tr("Add New Contact"), this);
    connect(actAddContact, SIGNAL(triggered()), SLOT(startAddContact()));
    actChangeAvatar = new QAction(QIcon(QLatin1String(":/icons/image--pencil.png")), tr("Change Avatar"), this);
    actOpenDownloads = new QAction(QIcon(QLatin1String(":/icons/folder-open-image.png")), tr("Open Downloads Folder"), this);

    actTestConnection = new QAction(QIcon(QLatin1String(":/icons/globe-green.png")), tr("Test Connection"), this);
    actOptions = new QAction(QIcon(QLatin1String(":/icons/gear.png")), tr("Options"), this);
    actTorConfig = new QAction(QIcon(QLatin1String(":/icons/wall--pencil.png")), tr("Configure Tor"), this);
    connect(actTorConfig, SIGNAL(triggered()), SLOT(startTorConfig()));

    QAction *separator = new QAction(this);
    separator->setSeparator(true);

    buttonActions << actAddContact << actChangeAvatar << actOpenDownloads;
    buttonActions << separator;
    buttonActions << actTestConnection << actOptions << actTorConfig;
}

#include <QGridLayout>

QLayout *HomeScreen::createButtons()
{
    QGridLayout *layout = new QGridLayout;
    layout->setSpacing(0);
    layout->setHorizontalSpacing(12);

    int row = 0, column = 0;

    QLabel *heading = new QLabel;
    heading->setPixmap(QPixmap(QLatin1String(":/graphics/logotext.png")));
    heading->setContentsMargins(0, 0, 0, 14);
    layout->addWidget(heading, row++, column, 1, 2, Qt::AlignTop | Qt::AlignHCenter);

    for (QList<QAction*>::ConstIterator it = buttonActions.begin(); it != buttonActions.end(); ++it)
    {
        if ((*it)->isSeparator())
        {
            //layout->addSpacing(8);
            //layout->addStretch();
            column++;
            row = 1;
            continue;
        }

        QToolButton *btn = new QToolButton;
        btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        btn->setFixedHeight(23);
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        btn->setAutoRaise(true);
        btn->setDefaultAction(*it);
        layout->addWidget(btn, row++, column, 1, 1, Qt::AlignTop | Qt::AlignLeft);
    }

    layout->setRowStretch(layout->rowCount()-1, 1);

    return layout;
}

QWidget *HomeScreen::createStatus()
{
    QWidget *widget = new QWidget;
    widget->setContextMenuPolicy(Qt::ActionsContextMenu);
    widget->addAction(actTestConnection);
    widget->addAction(actTorConfig);

    QBoxLayout *layout = new QHBoxLayout(widget);
    layout->setMargin(0);

    QFont font(QLatin1String("Calibri"));
    font.setPixelSize(13);

    QLabel *statusIcon = new QLabel;
    statusIcon->setPixmap(QPixmap(QLatin1String(":/icons/tick-circle.png")));
    statusIcon->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusIcon->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    layout->addWidget(statusIcon);

    torStatus = new QLabel;
    torStatus->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    torStatus->setFont(font);
    torStatus->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    layout->addWidget(torStatus);

    torInfo = new QLabel;
    torInfo->setTextFormat(Qt::PlainText);
    torInfo->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    font.setBold(true);
    torInfo->setFont(font);

    QPalette p = torInfo->palette();
    p.setColor(QPalette::WindowText, QColor(171, 171, 171));
    torInfo->setPalette(p);

    layout->addWidget(torInfo);

    connect(torManager, SIGNAL(statusChanged(int,int)), this, SLOT(updateTorStatus()));
    updateTorStatus();

    return widget;
}

void HomeScreen::updateTorStatus()
{
    QString infoText = QLatin1String("TorIM ") + QApplication::applicationVersion();

    QString torVersion = torManager->torVersion();
    if (!torVersion.isEmpty())
        infoText.append(QLatin1String("\nTor ") + torVersion);

    torInfo->setText(infoText);

    torStatus->setText(torManager->statusText());
}

void HomeScreen::startTorConfig()
{
    TorConfigWizard wizard(window());
    wizard.exec();
}

void HomeScreen::startAddContact()
{
    ContactAddDialog dialog(window());
    dialog.exec();
}
