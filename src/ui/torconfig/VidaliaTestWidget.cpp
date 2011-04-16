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

#include "VidaliaTestWidget.h"
#include "TorConnTestWidget.h"
#include "tor/autoconfig/VidaliaConfigManager.h"
#include <QBoxLayout>
#include <QLabel>

using namespace TorConfig;

VidaliaTestWidget::VidaliaTestWidget(VidaliaConfigManager *vc, QWidget *parent)
    : QWidget(parent), vidaliaConfig(vc)
{
    QBoxLayout *layout = new QVBoxLayout(this);

    /* Title */
    QLabel *title = new QLabel(tr("Testing the Vidalia configuration"));
    title->setStyleSheet(QLatin1String("font-size:13pt;"));
    title->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    layout->addWidget(title);
    layout->addSpacing(20);

    /* Test status */
    QBoxLayout *testLayout = new QHBoxLayout;
    layout->addLayout(testLayout);

    QLabel *testLabel = new QLabel(tr("Status:"));
    testLabel->setStyleSheet(QLatin1String("font-weight:bold"));
    testLayout->addWidget(testLabel);

    tester = new TorConnTestWidget;
    connect(tester, SIGNAL(stateChanged()), SLOT(testStateChanged()));
    testLayout->addWidget(tester, 1);

    layout->addStretch();

    startTest();
}

void VidaliaTestWidget::startTest()
{
    QString address;
    QByteArray password;
    quint16 port = 0;
    vidaliaConfig->getControlInfo(&address, &port, &password);

    tester->startTest(address, port, password);
}

bool VidaliaTestWidget::hasTestSucceeded() const
{
    return tester->hasTestSucceeded();
}

void VidaliaTestWidget::testStateChanged()
{
    emit stateChanged();
}
