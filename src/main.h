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

#ifndef MAIN_H
#define MAIN_H

#include <QtGlobal>
#include <QSettings>

class AppSettings : public QSettings
{
public:
    AppSettings(QObject *parent = 0)
        : QSettings(parent)
    {
    }

    AppSettings(const QString &filename, Format format, QObject *parent = 0)
        : QSettings(filename, format, parent)
    {
    }

    using QSettings::value;

    QVariant value(const char *key, const QVariant &defaultValue = QVariant()) const
    {
        return QSettings::value(QLatin1String(key), defaultValue);
    }

    using QSettings::setValue;

    void setValue(const char *key, const QVariant &value)
    {
        QSettings::setValue(QLatin1String(key), value);
    }

    using QSettings::remove;

    void remove(const char *key)
    {
        QSettings::remove(QLatin1String(key));
    }
};

extern AppSettings *config;

#endif // MAIN_H
