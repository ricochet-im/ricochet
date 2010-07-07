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

#include "main.h"
#include "TorConfigWizard.h"
#include "IntroPage.h"
#include "VidaliaConfigPage.h"
#include "ManualConfigPage.h"
#include <QMessageBox>

using namespace TorConfig;

TorConfigWizard::TorConfigWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("TorIM - Configure Tor"));
    setFixedSize(550, 450);

    addPage(new IntroPage);
    addPage(new VidaliaConfigPage);
    addPage(new ManualConfigPage);
}

void TorConfigWizard::accept()
{
    QString torMethod;
    switch (currentId())
    {
    case 1:
        torMethod = QLatin1String("vidalia");
        break;
    case 2:
        torMethod = QLatin1String("manual");
        break;
    }

    QString controlIp = field(QLatin1String("controlIp")).toString();
    quint16 controlPort = (quint16) field(QLatin1String("controlPort")).toUInt();

    if (torMethod.isEmpty() || controlIp.isEmpty() || controlPort < 1)
    {
        QMessageBox::critical(this, tr("Error"), tr("The wizard is incomplete; please go back and ensure all required "
                                                    "fields are filled"));
        return;
    }

    config->setValue("tor/configMethod", torMethod);
    config->setValue("tor/controlIp", controlIp);
    config->setValue("tor/controlPort", controlPort);

    QString authPassword = field(QLatin1String("controlPassword")).toString();
    if (!authPassword.isEmpty())
        config->setValue("tor/authPassword", authPassword);
    else
        config->remove("tor/authPassword");

    QWizard::accept();
}
