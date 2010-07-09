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

#include "IntroPage.h"
#include "TorConfigWizard.h"
#include "tor/autoconfig/VidaliaConfigManager.h"
#include "tor/autoconfig/BundledTorManager.h"
#include <QBoxLayout>
#include <QLabel>
#include <QCommandLinkButton>
#include <QSignalMapper>

using namespace TorConfig;

IntroPage::IntroPage(QWidget *parent)
    : QWizardPage(parent), bundleBtn(0), configChoice(-1)
{
    QBoxLayout *layout = new QVBoxLayout(this);

    /* Introduction */
    QLabel *intro = new QLabel;
    intro->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    intro->setWordWrap(true);
    intro->setText(tr(
        "First, Torsion must be configured to work with your installation of Tor. "
        "To continue, select your desired setup method below."
    ));
    layout->addWidget(intro);

    layout->addSpacing(40);

    QSignalMapper *btnMapper = new QSignalMapper(this);
    connect(btnMapper, SIGNAL(mapped(int)), this, SLOT(setConfigChoice(int)));

    /* Vidalia */
    vidaliaBtn = new QCommandLinkButton;
    vidaliaBtn->setText(tr("Use Vidalia (Recommended)"));
    vidaliaBtn->setDescription(tr("Automatically configure your existing Vidalia installation for Torsion"));

    if (!VidaliaConfigManager::isVidaliaInstalled())
    {
        vidaliaBtn->setEnabled(false);
        vidaliaBtn->setDescription(tr("If Vidalia is installed, it can be automatically configured to"
                                      " work with Torsion"));
    }

    layout->addWidget(vidaliaBtn);

    connect(vidaliaBtn, SIGNAL(clicked()), btnMapper, SLOT(map()));
    btnMapper->setMapping(vidaliaBtn, 1);

    /* Bundled Tor */
    if (BundledTorManager::isAvailable())
    {
        bundleBtn = new QCommandLinkButton;
        if (!vidaliaBtn->isEnabled())
        {
            bundleBtn->setText(tr("Use Bundled Tor (Recommended)"));
            vidaliaBtn->setText(tr("Use Vidalia"));
        }
        else
            bundleBtn->setText(tr("Use Bundled Tor"));

        bundleBtn->setDescription(tr("Automatically run and manage an included copy of Tor"));
        layout->addWidget(bundleBtn);

        connect(bundleBtn, SIGNAL(clicked()), SLOT(useBundled()));
    }

    /* Manual configuration */
    manualBtn = new QCommandLinkButton;
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

void IntroPage::useBundled()
{
    reinterpret_cast<TorConfigWizard*>(wizard())->accept(QLatin1String("bundle"));
}

void IntroPage::initializePage()
{
    wizard()->button(QWizard::NextButton)->setVisible(false);
    wizard()->button(QWizard::FinishButton)->setVisible(false);

    if (vidaliaBtn->isEnabled())
    {
        vidaliaBtn->setDefault(true);
        vidaliaBtn->setFocus();
    }
    else if (bundleBtn)
    {
        bundleBtn->setDefault(true);
        bundleBtn->setFocus();
    }
}

bool IntroPage::isComplete() const
{
    return false;
}

int IntroPage::nextId() const
{
    return configChoice;
}
