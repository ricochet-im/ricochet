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
