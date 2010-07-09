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

#ifndef BUNDLEDTORMANAGER_H
#define BUNDLEDTORMANAGER_H

#include <QObject>
#include <QProcess>

class BundledTorManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(BundledTorManager)

public:
    static BundledTorManager *instance();

    static bool isAvailable();

    bool isRunning() const { return process.state() == QProcess::Running; }

public slots:
    void start();

private:
    static BundledTorManager *m_instance;
    QProcess process;

    BundledTorManager();
};

#endif // BUNDLEDTORMANAGER_H
