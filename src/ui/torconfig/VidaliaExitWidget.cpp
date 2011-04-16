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
