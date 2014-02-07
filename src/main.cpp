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

#include "main.h"
#include "ui/MainWindow.h"
#include "core/IdentityManager.h"
#include "tor/TorManager.h"
#include "tor/TorControl.h"
#include "utils/CryptoKey.h"
#include "utils/SecureRNG.h"
#include <QApplication>
#include <QSettings>
#include <QTime>
#include <QDir>
#include <QTranslator>
#include <QMessageBox>
#include <QLocale>
#include <QLockFile>
#include <QStandardPaths>
#include <openssl/crypto.h>

AppSettings *config = 0;
static QLockFile *configLock = 0;

static void initSettings();
static void initTranslation();
static bool connectTorControl();

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setApplicationVersion(QLatin1String("0.2.0-dev"));

    initSettings();
    initTranslation();

    /* Initialize OpenSSL's allocator */
    CRYPTO_malloc_init();

    /* Seed the OpenSSL RNG */
    if (!SecureRNG::seed())
        qFatal("Failed to initialize RNG");
    qsrand(SecureRNG::randomInt(UINT_MAX));

    /* Tor control manager */
    torControl = Tor::TorManager::instance()->control();
    Tor::TorManager::instance()->start();

    /* Identities */
    identityManager = new IdentityManager;

    /* Window */
    MainWindow w;

    int r = a.exec();
    delete configLock;
    return r;
}

static QString userConfigPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
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

static void initSettings()
{
    /* The default QSettings logic is not desirable here. Instead, we do the following:
     * If the application directory is writable, it is preferred, except on OS X.
     *
     * On OS X, if the directory that contains the application bundle contains a directory
     * named, case-sensitively, 'Torsion.config', that folder will be used.
     *
     * Otherwise, a platform-specific per-user config location is used, generally matching the
     * QSettings user locations. To avoid odd behavior when a per-user config already
     * exists, the application directory is *always* used if possible. The file is always
     * named Torsion.ini. If a filename is given as a parameter, that will always be used
     * instead. */

    qApp->setOrganizationName(QLatin1String("Torsion"));

    QString configFile;
    QStringList args = qApp->arguments();
    if (args.size() > 1)
    {
        configFile = args[1];
    }
    else
    {
#ifndef Q_OS_MAC
        QString appDirFile = qApp->applicationDirPath() + QLatin1String("/Torsion.ini");
        if (!QFile::exists(appDirFile))
        {
            if (QFile(appDirFile).open(QIODevice::ReadWrite))
                configFile = appDirFile;
            else
                configFile = userConfigPath() + QLatin1String("/Torsion.ini");
        }
        else
            configFile = appDirFile;
#else
        QString portableLocation = appBundlePath() + QLatin1String("Torsion.config");
        QFileInfo fi(portableLocation);
        if (fi.exists() && fi.isDir())
            configFile = portableLocation + QLatin1String("/Torsion.ini");
        else
            configFile = userConfigPath() + QLatin1String("/Torsion.ini");
#endif
    }

    configLock = new QLockFile(configFile + QStringLiteral(".lock"));
    configLock->setStaleLockTime(0);
    if (!configLock->tryLock()) {
        if (configLock->error() == QLockFile::LockFailedError) {
            /* File is locked */
            QMessageBox::information(0, QApplication::tr("Torsion"),
                                     QApplication::tr("Torsion is already running."),
                                     QMessageBox::Ok);
            exit(0);
        } else {
            qWarning("Failed to acquire a lock on the configuration file");
        }
    }

    config = new AppSettings(configFile, QSettings::IniFormat);
    if (!config->isWritable())
        qWarning("Configuration file %s is not writable", qPrintable(configFile));

    QDir::setCurrent(QFileInfo(configFile).absoluteDir().path());
}

static void initTranslation()
{
    QTranslator *translator = new QTranslator;

    bool ok = false;
    QString appPath = qApp->applicationDirPath();
    QString resPath = QLatin1String(":/lang/");

    /* First, try to load the user's configured language */
    QString configLang = config->value(QLatin1String("core/language")).toString();
    if (!configLang.isEmpty())
    {
        QString filename = QLatin1String("torsion.") + configLang;
        QString separators = QLatin1String("_");

        /* Look in the application directory */
        ok = translator->load(filename, appPath, separators);
        /* Look in the resources */
        if (!ok)
            ok = translator->load(filename, resPath, separators);
    }

    /* Next, try to load the system locale language, and allow it to fall back to the english default */
    if (!ok)
    {
        QString filename = QLatin1String("torsion.") + QLocale::system().name();
        ok = translator->load(filename, appPath);
        if (!ok)
            ok = translator->load(filename, resPath);
    }

    if (ok)
        qApp->installTranslator(translator);
}

