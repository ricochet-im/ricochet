/* Torsion - http://torsionim.org/
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

#include "VidaliaExitWidget.h"
#include "tor/autoconfig/VidaliaConfigManager.h"
#include <QBoxLayout>
#include <QLabel>
#include <QTimer>

using namespace TorConfig;

VidaliaExitWidget::VidaliaExitWidget(VidaliaConfigManager *vc, QWidget *parent)
    : QWidget(parent), vidaliaConfig(vc)
{
    QBoxLayout *layout = new QVBoxLayout(this);

    QLabel *title = new QLabel(tr("To continue, you must exit Vidalia"));
    title->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    title->setStyleSheet(QLatin1String("font-size:13pt"));
    layout->addWidget(title);

    layout->addSpacing(20);

    QLabel *desc = new QLabel(tr("Look for the vidalia icon (<img src=':/graphics/vidalia-tray.png'>) in "
                                 "the system tray and choose to exit.<br>Anything currently using Tor (such "
                                 "as web downloads) will be interrupted."));
    desc->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    layout->addWidget(desc);
    layout->addSpacing(30);

    QLabel *desc2 = new QLabel(tr("Configuration will continue automatically when Vidalia is closed."));
    desc2->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    layout->addWidget(desc2);
    layout->addStretch();

    exitTimer = new QTimer(this);
    connect(exitTimer, SIGNAL(timeout()), SLOT(checkVidalia()));
    exitTimer->start(5000);
}

void VidaliaExitWidget::checkVidalia()
{
    if (!vidaliaConfig->isVidaliaRunning())
    {
        exitTimer->stop();
        emit exited();
    }
}
