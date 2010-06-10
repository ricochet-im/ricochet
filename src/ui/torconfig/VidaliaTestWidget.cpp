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
