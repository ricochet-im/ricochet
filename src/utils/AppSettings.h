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

#ifndef APPSETTINGS_H
#define APPSETTINGS_H

#include <QSettings>
#include <QMultiHash>

class AppSettings : public QSettings
{
    Q_OBJECT
    Q_DISABLE_COPY(AppSettings)

public:
    AppSettings(QObject *parent = 0)
        : QSettings(parent), keyChangeSource(0)
    {
    }

    AppSettings(const QString &filename, Format format, QObject *parent = 0)
        : QSettings(filename, format, parent), keyChangeSource(0)
    {
    }

    /* Convenience overloads that take const char*, to avoid the need for QLatin1String */
    using QSettings::value;
    QVariant value(const char *key, const QVariant &defaultValue = QVariant()) const
    {
        return QSettings::value(QLatin1String(key), defaultValue);
    }

    void setValue(const QString &key, const QVariant &value);
    void setValue(const char *key, const QVariant &value)
    {
        setValue(QString::fromLatin1(key), value);
    }

    void remove(const QString &key);
    void remove(const char *key)
    {
        remove(QString::fromLatin1(key));
    }

    /* Property synchronization (bidirectional; synchronizes the value of a property with a setting)
     * If the property does not have a notify signal, the connection will be unidirectional. */
    bool addTrackingProperty(const QString &key, QObject *object, const char *property = 0);

public slots:
    void removeTrackingProperty(QObject *object);

private slots:
    void keyChanged(const QString &key, const QVariant &value);
    void objectChanged(QObject *object, int index);

    /* I'm so sorry. At present (Qt 4.7), it is impossible to get the signal that emitted to the current
     * slot out of public API (though it is available in private API). This would be necessary to know
     * which property of an object changed. As a horrific hack, we define several slots, and connect to
     * each for a different property. */
    void objectChanged0() { objectChanged(sender(), 0); }
    void objectChanged1() { objectChanged(sender(), 1); }
    void objectChanged2() { objectChanged(sender(), 2); }
    void objectChanged3() { objectChanged(sender(), 3); }
    void objectChanged4() { objectChanged(sender(), 4); }

private:
    QMultiHash<QString,QObject*> trackingKeyMap;
    QMultiHash<QObject*,QPair<QString,int> > trackingObjectMap;
    QObject *keyChangeSource;
};

extern AppSettings *config;

#endif // APPSETTINGS_H
