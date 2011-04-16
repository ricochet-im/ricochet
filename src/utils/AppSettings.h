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

    QString configLocation() const;

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
