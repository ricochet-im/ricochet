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

#include "VidaliaConfigPage.h"
#include "VidaliaExitWidget.h"
#include "VidaliaStartWidget.h"
#include "VidaliaTestWidget.h"
#include "tor/autoconfig/VidaliaConfigManager.h"
#include <QStackedLayout>
#include <QBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVariant>
#include <QDebug>

using namespace TorConfig;

VidaliaConfigPage::VidaliaConfigPage(QWidget *parent)
    : QWizardPage(parent), m_stack(new QStackedLayout(this)), vidaliaConfig(0), testWidget(0)
{
}

void VidaliaConfigPage::initializePage()
{
    vidaliaConfig = new VidaliaConfigManager(this);
    Q_ASSERT(vidaliaConfig->isVidaliaInstalled());

    if (!vidaliaConfig->hasCompatibleConfig())
    {
        /* Reconfiguration will be necessary */
        if (vidaliaConfig->isVidaliaRunning())
        {
            /* Prompt the user to close Vidalia */
            VidaliaExitWidget *exitWidget = new VidaliaExitWidget(vidaliaConfig);
            connect(exitWidget, SIGNAL(exited()), this, SLOT(doConfiguration()));
            m_stack->setCurrentIndex(m_stack->addWidget(exitWidget));
        }
        else
            doConfiguration();
    }
    else
        doTest();
}

void VidaliaConfigPage::cleanupPage()
{
    while (m_stack->count())
        m_stack->takeAt(0)->widget()->deleteLater();

    delete vidaliaConfig;
    vidaliaConfig = 0;

    testWidget = 0;
}

bool VidaliaConfigPage::isComplete() const
{
    return testWidget && testWidget->hasTestSucceeded();
}

int VidaliaConfigPage::nextId() const
{
    return -1;
}

void VidaliaConfigPage::doConfiguration()
{
    /* Reconfigure */
    QString errorMessage;
    if (!vidaliaConfig->reconfigureControlConfig(&errorMessage))
    {
        QMessageBox::critical(window(), tr("Error"), tr("Vidalia reconfiguration error:\n\n%1").arg(errorMessage));
        wizard()->back();
        return;
    }

    /* Continue */
    doTest();
}

void VidaliaConfigPage::doTest()
{
    if (!vidaliaConfig->isVidaliaRunning())
    {
        /* Prompt the user to open Vidalia */
        bool wasExited = qobject_cast<VidaliaExitWidget*>(sender()) != 0;
        VidaliaStartWidget *startWidget = new VidaliaStartWidget(vidaliaConfig, wasExited);
        connect(startWidget, SIGNAL(ready()), this, SLOT(doTest()));
        m_stack->setCurrentIndex(m_stack->addWidget(startWidget));
        return;
    }

    /* Set fields */
    QString address;
    QByteArray password;
    quint16 port = 0;

    vidaliaConfig->getControlInfo(&address, &port, &password);

    setField(QLatin1String("controlIp"), address);
    setField(QLatin1String("controlPort"), port);
    setField(QLatin1String("controlPassword"), QString::fromLocal8Bit(password));

    /* Show test and results */
    Q_ASSERT(!testWidget);
    testWidget = new VidaliaTestWidget(vidaliaConfig);
    connect(testWidget, SIGNAL(stateChanged()), this, SIGNAL(completeChanged()));
    m_stack->setCurrentIndex(m_stack->addWidget(testWidget));
}
