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

#include "TorConnTestWidget.h"
#include "tor/TorControlManager.h"
#include <QBoxLayout>
#include <QLabel>

using namespace TorConfig;

TorConnTestWidget::TorConnTestWidget(QWidget *parent)
    : QWidget(parent), testManager(0), m_state(-1)
{
    QBoxLayout *layout = new QHBoxLayout(this);

    infoLabel = new QLabel;
    infoLabel->setText(tr("The connection has not been tested"));

    layout->addWidget(infoLabel);
}

void TorConnTestWidget::startTest(const QString &host, quint16 port, const QByteArray &authPassword)
{
    if (testManager)
    {
        testManager->disconnect(this);
        testManager->deleteLater();
    }

    m_state = -1;

    testManager = new Tor::TorControlManager(this);
    connect(testManager, SIGNAL(socksReady()), this, SLOT(doTestSuccess()));
    connect(testManager, SIGNAL(statusChanged(int,int)), this, SLOT(torStatusChanged(int)));

    testManager->setAuthPassword(authPassword);
    testManager->connect(QHostAddress(host), port);

    infoLabel->setText(tr("Testing connection..."));

    emit testStarted();
    emit stateChanged();
}

void TorConnTestWidget::clear()
{
    if (testManager)
    {
        testManager->disconnect(this);
        testManager->deleteLater();
        testManager = 0;
    }

    m_state = -1;
    infoLabel->setText(tr("The connection has not been tested"));
    emit stateChanged();
}

void TorConnTestWidget::doTestSuccess()
{
    infoLabel->setText(tr("Successfully connected with Tor %1").arg(testManager->torVersion()));
    testManager->deleteLater();
    testManager = 0;

    m_state = 1;
    emit testSucceeded();
    emit testFinished(true);
    emit stateChanged();
}

void TorConnTestWidget::doTestFail()
{
    infoLabel->setText(testManager->errorMessage());
    testManager->deleteLater();
    testManager = 0;

    m_state = 0;
    emit testFailed();
    emit testFinished(false);
    emit stateChanged();
}

void TorConnTestWidget::torStatusChanged(int status)
{
    if (status == Tor::TorControlManager::Error)
        doTestFail();
}
