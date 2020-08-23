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

#include "Settings.h"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSaveFile>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QDebug>
#include <QPointer>

class SettingsFilePrivate : public QObject
{
    Q_OBJECT

public:
    SettingsFile *q;
    QString filePath;
    QString errorMessage;
    QTimer syncTimer;
    QJsonObject jsonRoot;
    SettingsObject *rootObject;

    SettingsFilePrivate(SettingsFile *qp);
    virtual ~SettingsFilePrivate();

    void reset();
    void setError(const QString &message);
    bool checkDirPermissions(const QString &path);
    bool readFile();
    bool writeFile();

    static QStringList splitPath(const QString &input, bool &ok);
    QJsonValue read(const QJsonObject &base, const QStringList &path);
    bool write(const QStringList &path, const QJsonValue &value);

signals:
    void modified(const QStringList &path, const QJsonValue &value);

private slots:
    void sync();
};

SettingsFile::SettingsFile(QObject *parent)
    : QObject(parent), d(new SettingsFilePrivate(this))
{
    d->rootObject = new SettingsObject(this, QString());
}

SettingsFile::~SettingsFile()
{
}

SettingsFilePrivate::SettingsFilePrivate(SettingsFile *qp)
    : QObject(qp)
    , q(qp)
    , rootObject(0)
{
    syncTimer.setInterval(0);
    syncTimer.setSingleShot(true);
    connect(&syncTimer, &QTimer::timeout, this, &SettingsFilePrivate::sync);
}

SettingsFilePrivate::~SettingsFilePrivate()
{
    if (syncTimer.isActive())
        sync();
    delete rootObject;
}

void SettingsFilePrivate::reset()
{
    filePath.clear();
    errorMessage.clear();

    jsonRoot = QJsonObject();
    emit modified(QStringList(), jsonRoot);
}

QString SettingsFile::filePath() const
{
    return d->filePath;
}

bool SettingsFile::setFilePath(const QString &filePath)
{
    if (d->filePath == filePath)
        return hasError();

    d->reset();
    d->filePath = filePath;

    QFileInfo fileInfo(filePath);
    QDir dir(fileInfo.path());
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        d->setError(QStringLiteral("Cannot create directory: %1").arg(dir.path()));
        return false;
    }
    d->checkDirPermissions(fileInfo.path());

    if (!d->readFile())
        return false;

    return true;
}

QString SettingsFile::errorMessage() const
{
    return d->errorMessage;
}

bool SettingsFile::hasError() const
{
    return !d->errorMessage.isEmpty();
}

void SettingsFilePrivate::setError(const QString &message)
{
    errorMessage = message;
    emit q->error();
}

bool SettingsFilePrivate::checkDirPermissions(const QString &path)
{
    static QFile::Permissions desired = QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser;
    static QFile::Permissions ignored = QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner;

    QFile file(path);
    if ((file.permissions() & ~ignored) != desired) {
        qDebug() << "Correcting permissions on configuration directory";
        if (!file.setPermissions(desired)) {
            qWarning() << "Correcting permissions on configuration directory failed";
            return false;
        }
    }

    return true;
}

SettingsObject *SettingsFile::root()
{
    return d->rootObject;
}

const SettingsObject *SettingsFile::root() const
{
    return d->rootObject;
}

void SettingsFilePrivate::sync()
{
    if (filePath.isEmpty())
        return;

    syncTimer.stop();
    writeFile();
}

bool SettingsFilePrivate::readFile()
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadWrite)) {
        setError(file.errorString());
        return false;
    }

    QByteArray data = file.readAll();
    if (data.isEmpty() && (file.error() != QFileDevice::NoError || file.size() > 0)) {
        setError(file.errorString());
        return false;
    }

    if (data.isEmpty()) {
        jsonRoot = QJsonObject();
        return true;
    }

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(data, &parseError);
    if (document.isNull()) {
        setError(parseError.errorString());
        return false;
    }

    if (!document.isObject()) {
        setError(QStringLiteral("Invalid configuration file (expected object)"));
        return false;
    }

    jsonRoot = document.object();

    emit modified(QStringList(), jsonRoot);
    return true;
}

bool SettingsFilePrivate::writeFile()
{
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        setError(file.errorString());
        return false;
    }

    QJsonDocument document(jsonRoot);
    QByteArray data = document.toJson();
    if (data.isEmpty() && !document.isEmpty()) {
        setError(QStringLiteral("Encoding failure"));
        return false;
    }

    if (file.write(data) < data.size() || !file.commit()) {
        setError(file.errorString());
        return false;
    }

    return true;
}

QStringList SettingsFilePrivate::splitPath(const QString &input, bool &ok)
{
    QStringList components = input.split(QLatin1Char('.'));

    // Allow a leading '.' to simplify concatenation
    if (!components.isEmpty() && components.first().isEmpty())
        components.takeFirst();

    // No other empty components, including a trailing .
    foreach (const QString &word, components) {
        if (word.isEmpty()) {
            ok = false;
            return QStringList();
        }
    }

    ok = true;
    return components;
}

QJsonValue SettingsFilePrivate::read(const QJsonObject &base, const QStringList &path)
{
    QJsonValue current = base;

    foreach (const QString &key, path) {
        QJsonObject object = current.toObject();
        if (object.isEmpty() || (current = object.value(key)).isUndefined())
            return QJsonValue::Undefined;
    }

    return current;
}

// Compare two QJsonValue to find keys that have changed,
// recursing into objects and building paths as necessary.
typedef QList<QPair<QStringList, QJsonValue> > ModifiedList;
static void findModifiedRecursive(ModifiedList &modified, const QStringList &path, const QJsonValue &oldValue, const QJsonValue &newValue)
{
    if (oldValue.isObject() || newValue.isObject()) {
        // If either is a non-object type, this returns an empty object
        QJsonObject oldObject = oldValue.toObject();
        QJsonObject newObject = newValue.toObject();

        // Iterate keys of the original object and compare to new
        for (QJsonObject::iterator it = oldObject.begin(); it != oldObject.end(); it++) {
            QJsonValue newSubValue = newObject.value(it.key());
            if (*it == newSubValue)
                continue;

            if ((*it).isObject() || newSubValue.isObject())
                findModifiedRecursive(modified, QStringList() << path << it.key(), *it, newSubValue);
            else
                modified.append(qMakePair(QStringList() << path << it.key(), newSubValue));
        }

        // Iterate keys of the new object that may not be in original
        for (QJsonObject::iterator it = newObject.begin(); it != newObject.end(); it++) {
            if (oldObject.contains(it.key()))
                continue;

            if ((*it).isObject())
                findModifiedRecursive(modified, QStringList() << path << it.key(), QJsonValue::Undefined, it.value());
            else
                modified.append(qMakePair(QStringList() << path << it.key(), it.value()));
        }
    } else
        modified.append(qMakePair(path, newValue));
}

bool SettingsFilePrivate::write(const QStringList &path, const QJsonValue &value)
{
    typedef QVarLengthArray<QPair<QString,QJsonObject> > ObjectStack;
    ObjectStack stack;
    QJsonValue current = jsonRoot;
    QJsonValue originalValue;
    QString currentKey;

    foreach (const QString &key, path) {
        const QJsonObject &parent = current.toObject();
        stack.append(qMakePair(currentKey, parent));
        current = parent.value(key);
        currentKey = key;
    }

    // Stack now contains parent objects starting with the root, and current
    // is the old value. Write back changes in reverse.
    if (current == value)
        return false;
    originalValue = current;
    current = value;

    ObjectStack::const_iterator it = stack.end(), begin = stack.begin();
    while (it != begin) {
        --it;
        QJsonObject update = it->second;
        update.insert(currentKey, current);
        current = update;
        currentKey = it->first;
    }

    // current is now the updated jsonRoot
    jsonRoot = current.toObject();
    syncTimer.start();

    ModifiedList modified;
    findModifiedRecursive(modified, path, originalValue, value);

    for (ModifiedList::iterator it = modified.begin(); it != modified.end(); it++)
        emit this->modified(it->first, it->second);

    return true;
}

class SettingsObjectPrivate : public QObject
{
    Q_OBJECT

public:
    explicit SettingsObjectPrivate(SettingsObject *q);

    SettingsObject *q;
    SettingsFile *file;
    QStringList path;
    QJsonObject object;
    bool invalid;

    void setFile(SettingsFile *file);

public slots:
    void modified(const QStringList &absolutePath, const QJsonValue &value);
};

SettingsObject::SettingsObject(QObject *parent)
    : QObject(parent)
    , d(new SettingsObjectPrivate(this))
{
    d->setFile(defaultFile());
    if (d->file)
        setPath(QString());
}

SettingsObject::SettingsObject(const QString &path, QObject *parent)
    : QObject(parent)
    , d(new SettingsObjectPrivate(this))
{
    d->setFile(defaultFile());
    setPath(path);
}

SettingsObject::SettingsObject(SettingsFile *file, const QString &path, QObject *parent)
    : QObject(parent)
    , d(new SettingsObjectPrivate(this))
{
    d->setFile(file);
    setPath(path);
}

SettingsObject::SettingsObject(SettingsObject *base, const QString &path, QObject *parent)
    : QObject(parent)
    , d(new SettingsObjectPrivate(this))
{
    d->setFile(base->d->file);
    setPath(base->path() + QLatin1Char('.') + path);
}

SettingsObjectPrivate::SettingsObjectPrivate(SettingsObject *qp)
    : QObject(qp)
    , q(qp)
    , file(0)
    , invalid(true)
{
}

void SettingsObjectPrivate::setFile(SettingsFile *value)
{
    if (file == value)
        return;

    if (file)
        disconnect(file, 0, this, 0);
    file = value;
    if (file)
        connect(file->d, &SettingsFilePrivate::modified, this, &SettingsObjectPrivate::modified);
}

// Emit SettingsObject::modified with a relative path if path is matched
void SettingsObjectPrivate::modified(const QStringList &key, const QJsonValue &value)
{
    if (key.size() < path.size())
        return;

    for (int i = 0; i < path.size(); i++) {
        if (path[i] != key[i])
            return;
    }

    object = file->d->read(file->d->jsonRoot, path).toObject();
    emit q->modified(QStringList(key.mid(path.size())).join(QLatin1Char('.')), value);
    emit q->dataChanged();
}

static QPointer<SettingsFile> defaultObjectFile;

SettingsFile *SettingsObject::defaultFile()
{
    return defaultObjectFile;
}

void SettingsObject::setDefaultFile(SettingsFile *file)
{
    defaultObjectFile = file;
}

QString SettingsObject::path() const
{
    return d->path.join(QLatin1Char('.'));
}

void SettingsObject::setPath(const QString &input)
{
    bool ok = false;
    QStringList newPath = SettingsFilePrivate::splitPath(input, ok);
    if (!ok) {
        d->invalid = true;
        d->path.clear();
        d->object = QJsonObject();

        emit pathChanged();
        emit dataChanged();
        return;
    }

    if (!d->invalid && d->path == newPath)
        return;

    d->path = newPath;
    if (d->file) {
        d->invalid = false;
        d->object = d->file->d->read(d->file->d->jsonRoot, d->path).toObject();
        emit dataChanged();
    }

    emit pathChanged();
}

QJsonObject SettingsObject::data() const
{
    return d->object;
}

void SettingsObject::setData(const QJsonObject &input)
{
    if (d->invalid || d->object == input)
        return;

    d->object = input;
    d->file->d->write(d->path, d->object);
}

QJsonValue SettingsObject::read(const QString &key, const QJsonValue &defaultValue) const
{
    bool ok = false;
    QStringList splitKey = SettingsFilePrivate::splitPath(key, ok);
    if (d->invalid || !ok || splitKey.isEmpty()) {
        qDebug() << "Invalid settings read of path" << key;
        return defaultValue;
    }

    QJsonValue ret = d->file->d->read(d->object, splitKey);
    if (ret.isUndefined())
        ret = defaultValue;
    return ret;
}

void SettingsObject::write(const QString &key, const QJsonValue &value)
{
    bool ok = false;
    QStringList splitKey = SettingsFilePrivate::splitPath(key, ok);
    if (d->invalid || !ok || splitKey.isEmpty()) {
        qDebug() << "Invalid settings write of path" << key;
        return;
    }

    splitKey = d->path + splitKey;
    d->file->d->write(splitKey, value);
}

void SettingsObject::unset(const QString &key)
{
    write(key, QJsonValue());
}

void SettingsObject::undefine()
{
    if (d->invalid)
        return;

    d->object = QJsonObject();
    d->file->d->write(d->path, QJsonValue::Undefined);
}

#include "Settings.moc"
