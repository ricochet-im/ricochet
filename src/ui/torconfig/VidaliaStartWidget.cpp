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
