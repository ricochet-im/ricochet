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

#ifndef INTROPAGE_H
#define INTROPAGE_H

#include <QWizardPage>

namespace TorConfig
{

class IntroPage : public QWizardPage
{
    Q_OBJECT
    Q_DISABLE_COPY(IntroPage)

public:
    explicit IntroPage(QWidget *parent = 0);

    virtual void initializePage();

    virtual bool isComplete() const;
    virtual int nextId() const;

private slots:
    void setConfigChoice(int choice);

private:
    int configChoice;
};

}

#endif // INTROPAGE_H
