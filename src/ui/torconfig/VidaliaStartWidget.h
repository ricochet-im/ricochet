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

#ifndef VIDALIASTARTWIDGET_H
#define VIDALIASTARTWIDGET_H

#include <QWidget>

class VidaliaConfigManager;
class QTimer;
class QTcpSocket;

namespace TorConfig
{

class VidaliaStartWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(VidaliaStartWidget)

public:
    explicit VidaliaStartWidget(VidaliaConfigManager *vidaliaConfig, bool restarting, QWidget *parent = 0);

public slots:
    void checkTorActive();

signals:
    void ready();

private slots:
    void torReady();

private:
    VidaliaConfigManager * const vidaliaConfig;
    QTimer *testTimer;
    QTcpSocket *testSocket;
};

}

#endif // VIDALIASTARTWIDGET_H
