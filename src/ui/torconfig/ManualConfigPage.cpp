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

#include "ManualConfigPage.h"
#include "TorConnTestWidget.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QIntValidator>
#include <QPushButton>
#include <QVariant>

using namespace TorConfig;

ManualConfigPage::ManualConfigPage(QWidget *parent)
    : QWizardPage(parent)
{
    setButtonText(QWizard::CustomButton1, tr("Verify Connection"));

    QBoxLayout *layout = new QVBoxLayout(this);

    QLabel *desc = new QLabel;
    desc->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    desc->setWordWrap(true);
    desc->setTextFormat(Qt::RichText);
    desc->setText(tr(
        "TorIM requires a Tor controller connection instead of a normal proxy connection. "
        "This is configured with the <i>ControlPort</i> and <i>HashedControlPassword</i> options in the "
        "Tor configuration. You must set these options in your Tor configuration, and input them here."
    ));

    layout->addWidget(desc);
    layout->addSpacing(20);

    QFormLayout *formLayout = new QFormLayout;
    layout->addLayout(formLayout);

    /* Test widget */
    torTest = new TorConnTestWidget;
    connect(torTest, SIGNAL(stateChanged()), this, SIGNAL(completeChanged()));

    /* IP */
    ipEdit = new QLineEdit;
    ipEdit->setWhatsThis(tr("The IP of the Tor control connection"));

    QRegExpValidator *validator = new QRegExpValidator(QRegExp(QString(
            "^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$")),
            ipEdit);
    ipEdit->setValidator(validator);

    registerField(QString("controlIp*"), ipEdit);
    formLayout->addRow(tr("Control IP"), ipEdit);

    connect(ipEdit, SIGNAL(textChanged(QString)), torTest, SLOT(clear()));

    /* Port */
    portEdit = new QLineEdit;
    portEdit->setValidator(new QIntValidator(1, 65535, portEdit));
    portEdit->setWhatsThis(tr("The port used for the Tor control connection (ControlPort option)"));

    registerField(QString("controlPort*"), portEdit);
    formLayout->addRow(tr("Control Port"), portEdit);

    connect(portEdit, SIGNAL(textChanged(QString)), torTest, SLOT(clear()));

    /* Password */
    QLineEdit *passwordEdit = new QLineEdit;
    passwordEdit->setWhatsThis(tr("The password for control authentication. Plaintext of the "
                                  "HashedControlPassword option in Tor."));

    registerField(QString("controlPassword"), passwordEdit);
    formLayout->addRow(tr("Control Password"), passwordEdit);

    connect(passwordEdit, SIGNAL(textChanged(QString)), torTest, SLOT(clear()));

    /* Tester */
    QBoxLayout *testLayout = new QHBoxLayout;

    testLayout->addWidget(torTest, 1, Qt::AlignVCenter | Qt::AlignLeft);

    QPushButton *testBtn = new QPushButton(tr("Test Connection"));
    testLayout->addWidget(testBtn, 0, Qt::AlignVCenter | Qt::AlignRight);

    connect(testBtn, SIGNAL(clicked()), this, SLOT(testSettings()));

    layout->addStretch();
    layout->addLayout(testLayout);
    layout->addStretch();
}

void ManualConfigPage::initializePage()
{
    ipEdit->setText(QString("127.0.0.1"));
    portEdit->setText(QString("9051"));
}

bool ManualConfigPage::isComplete() const
{
    return torTest->hasTestSucceeded();
}

void ManualConfigPage::testSettings()
{
    torTest->startTest(field(QString("controlIp")).toString(), field("controlPort").toString().toInt(),
                       field("controlPassword").toString().toLocal8Bit());
}
