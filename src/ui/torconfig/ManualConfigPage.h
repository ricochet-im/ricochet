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

#ifndef MANUALCONFIGPAGE_H
#define MANUALCONFIGPAGE_H

#include <QWizardPage>

class QLineEdit;

namespace TorConfig
{

class ManualConfigPage : public QWizardPage
{
    Q_OBJECT
    Q_DISABLE_COPY(ManualConfigPage)

public:
    explicit ManualConfigPage(QWidget *parent = 0);

    virtual void initializePage();

    virtual bool isComplete() const;

public slots:
    void testSettings();

private:
    QLineEdit *ipEdit, *portEdit;
    class TorConnTestWidget *torTest;
};

}

#endif // MANUALCONFIGPAGE_H
