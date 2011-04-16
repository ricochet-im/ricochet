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

#include "VidaliaStartWidget.h"
#include "tor/autoconfig/VidaliaConfigManager.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QTcpSocket>

using namespace TorConfig;

VidaliaStartWidget::VidaliaStartWidget(VidaliaConfigManager *vc, bool restarting, QWidget *parent)
    : QWidget(parent), vidaliaConfig(vc), testSocket(new QTcpSocket(this))
{
    QBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel;
    if (restarting)
        title->setText(tr("Configuration done - please restart Vidalia"));
    else
        title->setText(tr("To continue, you must start Vidalia"));
    title->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    title->setStyleSheet(QLatin1String("font-size:13pt;"));
    layout->addWidget(title);
    layout->addSpacing(20);

#ifdef Q_OS_WIN
    QLabel *desc = new QLabel(tr("Vidalia should be available in your start menu or as an icon on your desktop."));
#else
    QLabel *desc = new QLabel(tr("Vidalia should be available in your applications menu."));
#endif
    desc->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    layout->addWidget(desc);
    layout->addSpacing(30);

    QLabel *desc2 = new QLabel(tr("Configuration will continue automatically when Vidalia and Tor have started."));
    desc2->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    layout->addWidget(desc2);
    layout->addStretch();

    connect(testSocket, SIGNAL(connected()), SLOT(torReady()));

    testTimer = new QTimer(this);
    connect(testTimer, SIGNAL(timeout()), SLOT(checkTorActive()));
    testTimer->start(5000);
}

void VidaliaStartWidget::checkTorActive()
{
    if (!vidaliaConfig->isVidaliaRunning())
        return;

    testSocket->abort();

    QString address;
    quint16 port = 0;
    vidaliaConfig->getControlInfo(&address, &port);

    testSocket->connectToHost(address, port);
}

void VidaliaStartWidget::torReady()
{
    testSocket->abort();
    testSocket->deleteLater();
    testSocket = 0;

    testTimer->stop();

    emit ready();
}
