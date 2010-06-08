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

#ifndef VIDALIACONFIGMANAGER_H
#define VIDALIACONFIGMANAGER_H

#include <QObject>

class VidaliaConfigManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(VidaliaConfigManager)

public:
    explicit VidaliaConfigManager(QObject *parent = 0);

    static bool isVidaliaInstalled();
    static QString vidaliaConfigPath();

    QString path() const { return m_path; }

    qint64 currentPid() const;
    bool isVidaliaRunning() const;

    /* True if Vidalia is setup in a way that we can use without reconfiguraiton */
    bool hasCompatibleConfig() const;

private:
    QString m_path;
};

#endif // VIDALIACONFIGMANAGER_H
