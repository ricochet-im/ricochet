/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>
#include <QDateTime>

class SettingsObject;
class SettingsFilePrivate;
class SettingsObjectPrivate;

/* SettingsFile represents a JSON-encoded configuration file.
 *
 * SettingsFile is an API for reading, writing, and change notification
 * on JSON-encoded settings files.
 *
 * Data is accessed via SettingsObject, either using the root property
 * or by creating a SettingsObject, optionally using a base path.
 */
class SettingsFile : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsFile)

    Q_PROPERTY(SettingsObject *root READ root CONSTANT)
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY error)
    Q_PROPERTY(bool hasError READ hasError NOTIFY error)

public:
    explicit SettingsFile(QObject *parent = 0);
    virtual ~SettingsFile();

    QString filePath() const;
    bool setFilePath(const QString &filePath);

    QString errorMessage() const;
    bool hasError() const;

    SettingsObject *root();
    const SettingsObject *root() const;

signals:
    void filePathChanged();
    void error();

private:
    SettingsFilePrivate *d;

    friend class SettingsObject;
    friend class SettingsObjectPrivate;
};

/* SettingsObject reads and writes data within a SettingsFile
 *
 * A SettingsObject is associated with a SettingsFile and represents an object
 * tree within that file. It refers to the JSON object tree using a path
 * notation with keys separated by '.'. For example:
 *
 * {
 *     "one": {
 *        "two": {
 *            "three": "value"
 *         }
 *     }
 * }
 *
 * With this data, a SettingsObject with an empty path can read with the path
 * "one.two.three", and a SettingsObject with a path of "one.two" can simply
 * read or write on "three".
 *
 * Multiple SettingsObjects may be created for the same path, and will be kept
 * synchronized with changes. The modified signal is emitted for all changes
 * affecting keys within a path, including writes of object trees and from other
 * instances.
 */
class SettingsObject : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(SettingsObject)

    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QJsonObject data READ data WRITE setData NOTIFY dataChanged)

public:
    explicit SettingsObject(QObject *parent = 0);
    explicit SettingsObject(const QString &path, QObject *parent = 0);
    explicit SettingsObject(SettingsFile *file, const QString &path, QObject *parent = 0);
    explicit SettingsObject(SettingsObject *base, const QString &path, QObject *parent = 0);

    /* Specify a SettingsFile to use by default on SettingsObject instances.
     *
     * After calling setDefaultFile, a SettingsObject created without any file, e.g.:
     *
     *     SettingsObject settings;
     *     SettingsObject animals(QStringLiteral("animals"));
     *
     * Will use the specified SettingsFile instance by default. This is a convenience
     * over passing around instances of SettingsFile in application use cases, and is
     * particularly useful for QML.
     */
    static SettingsFile *defaultFile();
    static void setDefaultFile(SettingsFile *file);

    QString path() const;
    void setPath(const QString &path);

    QJsonObject data() const;
    void setData(const QJsonObject &data);

    Q_INVOKABLE QJsonValue read(const QString &key, const QJsonValue &defaultValue = QJsonValue::Undefined) const;
    template<typename T> T read(const QString &key) const;
    Q_INVOKABLE void write(const QString &key, const QJsonValue &value);
    template<typename T> void write(const QString &key, const T &value);
    Q_INVOKABLE void unset(const QString &key);

    // const char* key overloads
    QJsonValue read(const char *key, const QJsonValue &defaultValue = QJsonValue::Undefined) const
    {
        return read(QString::fromLatin1(key), defaultValue);
    }
    template<typename T> T read(const char *key) const
    {
        return read<T>(QString::fromLatin1(key));
    }
    void write(const char *key, const QJsonValue &value)
    {
        write(QString::fromLatin1(key), value);
    }
    template<typename T> void write(const char *key, const T &value)
    {
        write<T>(QString::fromLatin1(key), value);
    }
    void unset(const char *key)
    {
        unset(QString::fromLatin1(key));
    }

    Q_INVOKABLE void undefine();

signals:
    void pathChanged();
    void dataChanged();

    void modified(const QString &path, const QJsonValue &value);

private:
    SettingsObjectPrivate *d;
};

template<typename T> inline void SettingsObject::write(const QString &key, const T &value)
{
    write(key, QJsonValue(value));
}

template<> inline QString SettingsObject::read<QString>(const QString &key) const
{
    return read(key).toString();
}

template<> inline QJsonArray SettingsObject::read<QJsonArray>(const QString &key) const
{
    return read(key).toArray();
}

template<> inline QJsonObject SettingsObject::read<QJsonObject>(const QString &key) const
{
    return read(key).toObject();
}

template<> inline double SettingsObject::read<double>(const QString &key) const
{
    return read(key).toDouble();
}

template<> inline int SettingsObject::read<int>(const QString &key) const
{
    return read(key).toInt();
}

template<> inline bool SettingsObject::read<bool>(const QString &key) const
{
    return read(key).toBool();
}

template<> inline QDateTime SettingsObject::read<QDateTime>(const QString &key) const
{
    QString value = read(key).toString();
    if (value.isEmpty())
        return QDateTime();
    return QDateTime::fromString(value, Qt::ISODate).toLocalTime();
}

template<> inline void SettingsObject::write<QDateTime>(const QString &key, const QDateTime &value)
{
    write(key, QJsonValue(value.toUTC().toString(Qt::ISODate)));
}

// Explicitly store value encoded as base64. Decodes and casts implicitly to QByteArray for reads.
class Base64Encode
{
public:
    explicit Base64Encode(const QByteArray &value) : d(value) { }
    operator QByteArray() { return d; }
    QByteArray encoded() const { return d.toBase64(); }

private:
    QByteArray d;
};

template<> inline Base64Encode SettingsObject::read<Base64Encode>(const QString &key) const
{
    return Base64Encode(QByteArray::fromBase64(read(key).toString().toLatin1()));
}

template<> inline void SettingsObject::write<Base64Encode>(const QString &key, const Base64Encode &value)
{
    write(key, QJsonValue(QString::fromLatin1(value.encoded())));
}

#endif

