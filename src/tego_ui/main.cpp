/* Ricochet Refresh - https://ricochetrefresh.net/
 * Copyright (C) 2019, Blueprint For Free Speech <ricochet@blueprintforfreespeech.net>
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

#include "ui/MainWindow.h"

// TODO: these includes need to go
#include "core/IdentityManager.h"
#include "tor/TorManager.h"
#include "tor/TorControl.h"
#include "utils/CryptoKey.h"
#include "utils/Settings.h"

#include <libtego_callbacks.hpp>

static bool initSettings(SettingsFile *settings, QLockFile **lockFile, QString &errorMessage);
static void initTranslation();

int main(int argc, char *argv[]) try
{
   /* Disable rwx memory.
       This will also ensure full PAX/Grsecurity protections. */
    qputenv("QV4_FORCE_INTERPRETER",  "1");
    qputenv("QT_ENABLE_REGEXP_JIT",   "0");
    /* Use QtQuick 2D renderer by default; ignored if not available */
    if (qEnvironmentVariableIsEmpty("QMLSCENE_DEVICE"))
        qputenv("QMLSCENE_DEVICE", "softwarecontext");

    /* https://doc.qt.io/qt-5/highdpi.html#high-dpi-support-in-qt */
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    if (!qEnvironmentVariableIsSet("QT_DEVICE_PIXEL_RATIO")
            && !qEnvironmentVariableIsSet("QT_AUTO_SCREEN_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCALE_FACTOR")
            && !qEnvironmentVariableIsSet("QT_SCREEN_SCALE_FACTORS")) {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }

    QApplication a(argc, argv);

    tego_context_t* tegoContext = nullptr;
    tego_initialize(&tegoContext, tego::throw_on_error());

    auto tego_cleanup = tego::make_scope_exit([=]() -> void {
        tego_uninitialize(tegoContext, tego::throw_on_error());
    });

    init_libtego_callbacks(tegoContext);

#ifdef TEGO_VERSION
#   define XSTR(X) STR(X)
#   define STR(X) #X
#   define TEGO_VERSION_STR XSTR(TEGO_VERSION)
#else
#   define TEGO_VERSION_STR "devbuild"
#endif
    a.setApplicationVersion(QLatin1String(TEGO_VERSION_STR));
    a.setOrganizationName(QStringLiteral("Ricochet"));

#if !defined(Q_OS_WIN) && !defined(Q_OS_MAC)
    a.setWindowIcon(QIcon(QStringLiteral(":/icons/ricochet_refresh.svg")));
#endif

    QScopedPointer<SettingsFile> settings(new SettingsFile);
    SettingsObject::setDefaultFile(settings.data());

    QString error;
    QLockFile *lock = 0;
    if (!initSettings(settings.data(), &lock, error)) {
        QMessageBox::critical(0, qApp->translate("Main", "Ricochet Error"), error);
        return 1;
    }
    QScopedPointer<QLockFile> lockFile(lock);

    initTranslation();



    /* Seed the OpenSSL RNG */


    /* Tor control manager */
    Tor::TorManager *torManager = Tor::TorManager::instance();
    torManager->setDataDirectory(QFileInfo(settings->filePath()).path() + QStringLiteral("/tor/"));
    torControl = torManager->control();
    torManager->start();

    /* Identities */
    // const auto createNewIdentity = (SettingsObject().read("identity") == QJsonValue::Undefined);
    QString serviceID = []()
    {
        auto settings = SettingsObject(QStringLiteral("identity"), nullptr);
        return settings.read<QString>("serviceKey");
    }();

    QVector<QString> contactHostnames = []()
    {
        auto settings = SettingsObject(QStringLiteral("contacts"));
        QVector<QString> retval;

        foreach (const QString &serviceId, settings.data().keys())
        {
            auto currentHostname = QString(serviceId).append(".onion");
            retval.push_back(currentHostname);
        }

        return retval;
    }();

    identityManager = new IdentityManager(serviceID, contactHostnames);
    QScopedPointer<IdentityManager> scopedIdentityManager(identityManager);

    /* Window */
    QScopedPointer<MainWindow> w(new MainWindow);
    if (!w->showUI())
        return 1;

    return a.exec();
}
catch(std::exception& re)
{
    qDebug() << "Caught Exception: " << re.what();
    return -1;
}


static QString userConfigPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QString oldPath = path;
    oldPath.replace(QStringLiteral("Ricochet"), QStringLiteral("Torsion"), Qt::CaseInsensitive);
    if (QFile::exists(oldPath))
        return oldPath;
    return path;
}

#ifdef Q_OS_MAC
static QString appBundlePath()
{
    QString path = QApplication::applicationDirPath();
    int p = path.lastIndexOf(QLatin1String(".app/"));
    if (p >= 0)
    {
        p = path.lastIndexOf(QLatin1Char('/'), p);
        path = path.left(p+1);
    }

    return path;
}
#endif

// Writes default settings to settings object. Does not care about any
// preexisting values, therefore this is best used on a fresh object.
static void loadDefaultSettings(SettingsFile *settings)
{
    settings->root()->write("ui.combinedChatWindow", true);
}

static bool initSettings(SettingsFile *settings, QLockFile **lockFile, QString &errorMessage)
{
    /* If built in portable mode (default), configuration is stored in the 'config'
     * directory next to the binary. If not writable, launching fails.
     *
     * Portable OS X is an exception. In that case, configuration is stored in a
     * 'config.ricochet' folder next to the application bundle, unless the application
     * path contains "/Applications", in which case non-portable mode is used.
     *
     * When not in portable mode, a platform-specific per-user config location is used.
     *
     * This behavior may be overriden by passing a folder path as the first argument.
     */

    QString configPath;
    QStringList args = qApp->arguments();
    if (args.size() > 1) {
        configPath = args[1];
    } else {
#ifndef RICOCHET_NO_PORTABLE
# ifdef Q_OS_MAC
        if (!qApp->applicationDirPath().contains(QStringLiteral("/Applications"))) {
            // Try old configuration path first
            configPath = appBundlePath() + QStringLiteral("config.torsion");
            if (!QFile::exists(configPath))
                configPath = appBundlePath() + QStringLiteral("config.ricochet");
        }
# else
        configPath = qApp->applicationDirPath() + QStringLiteral("/config");
# endif
#endif
        if (configPath.isEmpty())
            configPath = userConfigPath();
    }

    QDir dir(configPath);
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        errorMessage = QStringLiteral("Cannot create directory: %1").arg(dir.path());
        return false;
    }

    // Reset to config directory for consistency; avoid depending on this behavior for paths
    if (QDir::setCurrent(dir.absolutePath()) && dir.isRelative())
        dir.setPath(QStringLiteral("."));

    QLockFile *lock = new QLockFile(dir.filePath(QStringLiteral("ricochet.json.lock")));
    *lockFile = lock;
    lock->setStaleLockTime(0);
    if (!lock->tryLock()) {
        if (lock->error() == QLockFile::LockFailedError) {
            // This happens if a stale lock file exists and another process uses that PID.
            // Try removing the stale file, which will fail if a real process is holding a
            // file-level lock. A false error is more problematic than not locking properly
            // on corner-case systems.
            if (!lock->removeStaleLockFile() || !lock->tryLock()) {
                errorMessage = QStringLiteral("Configuration file is already in use");
                return false;
            } else
                qDebug() << "Removed stale lock file";
        } else {
            errorMessage = QStringLiteral("Cannot write configuration file (failed to acquire lock)");
            return false;
        }
    }

    settings->setFilePath(dir.filePath(QStringLiteral("ricochet.json")));
    if (settings->hasError()) {
        errorMessage = settings->errorMessage();
        return false;
    }

    // if still empty, load defaults here
    if (settings->root()->data().isEmpty()) {
        loadDefaultSettings(settings);
    }

    return true;
}

static void initTranslation()
{
    QTranslator *translator = new QTranslator;

    bool ok = false;
    QString appPath = qApp->applicationDirPath();
    QString resPath = QLatin1String(":/lang/");

    QLocale locale = QLocale::system();
    if (!qgetenv("RICOCHET_LOCALE").isEmpty()) {
        locale = QLocale(QString::fromLatin1(qgetenv("RICOCHET_LOCALE")));
        qDebug() << "Forcing locale" << locale << "from environment" << locale.uiLanguages();
    }

    SettingsObject settings;
    QString settingsLanguage(settings.read("ui.language").toString());

    if (!settingsLanguage.isEmpty()) {
        locale = settingsLanguage;
    } else {
        //write an empty string to get "System default" language selected automatically in preferences
        settings.write(QStringLiteral("ui.language"), QString());
    }

    ok = translator->load(locale, QStringLiteral("ricochet"), QStringLiteral("_"), appPath);
    if (!ok)
        ok = translator->load(locale, QStringLiteral("ricochet"), QStringLiteral("_"), resPath);

    if (ok) {
        qApp->installTranslator(translator);

        QTranslator *qtTranslator = new QTranslator;
        ok = qtTranslator->load(QStringLiteral("qt_") + locale.name(), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
        if (ok)
            qApp->installTranslator(qtTranslator);
        else
            delete qtTranslator;
    } else
        delete translator;
}

