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

#include "IntroPage.h"
#include <QBoxLayout>
#include <QLabel>
#include <QCommandLinkButton>
#include <QSignalMapper>

using namespace TorConfig;

IntroPage::IntroPage(QWidget *parent)
    : QWizardPage(parent), configChoice(-1)
{
    QBoxLayout *layout = new QVBoxLayout(this);

    /* Introduction */
    QLabel *intro = new QLabel;
    intro->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    intro->setWordWrap(true);
    intro->setText(tr(
        "First, TorIM must be configured to work with your installation of Tor. "
        "To continue, select your desired setup method below."
    ));
    layout->addWidget(intro);

    layout->addSpacing(40);

    QSignalMapper *btnMapper = new QSignalMapper(this);
    connect(btnMapper, SIGNAL(mapped(int)), this, SLOT(setConfigChoice(int)));

    /* Vidalia (temporary) */
    QCommandLinkButton *vidaliaBtn = new QCommandLinkButton;
    vidaliaBtn->setText(tr("Use Vidalia (Recommended)"));
    vidaliaBtn->setDescription(tr("Automatically reconfigure Vidalia and Tor to work with TorIM"));

    connect(vidaliaBtn, SIGNAL(clicked()), btnMapper, SLOT(map()));
    btnMapper->setMapping(vidaliaBtn, 1);

    layout->addWidget(vidaliaBtn);

    /* Manual configuration */
    QCommandLinkButton *manualBtn = new QCommandLinkButton;
    manualBtn->setText(tr("Configure manually"));
    manualBtn->setDescription(tr("Manually setup the Tor control connection. Advanced users only."));
    layout->addWidget(manualBtn);

    connect(manualBtn, SIGNAL(clicked()), btnMapper, SLOT(map()));
    btnMapper->setMapping(manualBtn, 2);

    layout->addStretch();
}

void IntroPage::setConfigChoice(int choice)
{
    configChoice = choice;
    wizard()->next();
}

void IntroPage::initializePage()
{
    wizard()->button(QWizard::NextButton)->setVisible(false);
    wizard()->button(QWizard::FinishButton)->setVisible(false);
}

bool IntroPage::isComplete() const
{
    return false;
}

int IntroPage::nextId() const
{
    return configChoice;
}
