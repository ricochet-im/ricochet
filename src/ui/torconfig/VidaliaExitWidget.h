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

#ifndef VIDALIAEXITWIDGET_H
#define VIDALIAEXITWIDGET_H

#include <QWidget>

class VidaliaConfigManager;
class QTimer;

namespace TorConfig
{

class VidaliaExitWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VidaliaExitWidget)

public:
    explicit VidaliaExitWidget(VidaliaConfigManager *vidaliaConfig, QWidget *parent = 0);

public slots:
    void checkVidalia();

signals:
    void exited();

private:
    VidaliaConfigManager * const vidaliaConfig;
    QTimer *exitTimer;
};

}

#endif // VIDALIAEXITWIDGET_H
