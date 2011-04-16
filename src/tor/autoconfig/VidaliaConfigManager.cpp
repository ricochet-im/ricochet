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

#include "VidaliaConfigManager.h"
#include "utils/OSUtil.h"
#include <QFile>
#include <QDir>
#include <QLibrary>
#include <QSettings>
#include <QtDebug>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <ShlObj.h>
#endif

VidaliaConfigManager::VidaliaConfigManager(QObject *parent)
    : QObject(parent), m_path(vidaliaConfigPath())
{
}

bool VidaliaConfigManager::isVidaliaInstalled()
{
    QString path = vidaliaConfigPath();
    if (path.isEmpty())
        return false;

    return QFile::exists(path + QLatin1String("/vidalia.conf"));
}

QString VidaliaConfigManager::vidaliaConfigPath()
{
#ifdef Q_OS_WIN
    QLibrary library(QLatin1String("shell32"));
    typedef BOOL (WINAPI*GetSpecialFolderPath)(HWND, LPWSTR, int, BOOL);
    static GetSpecialFolderPath SHGetSpecialFolderPath =
            (GetSpecialFolderPath)library.resolve("SHGetSpecialFolderPathW");

    QString path;
    wchar_t buf[MAX_PATH];

    if (SHGetSpecialFolderPath && SHGetSpecialFolderPath(0, buf, CSIDL_APPDATA, FALSE))
        path = QString::fromWCharArray(buf);
    else
        path = QDir::homePath() + QLatin1String("\\Application Data");

    path += QLatin1String("\\Vidalia");
    return path;
#elif defined(Q_OS_MAC)
    return QDir::homePath() + QLatin1String("/Library/Vidalia");
#else
    return QDir::homePath() + QLatin1String("/.vidalia");
#endif
}

qint64 VidaliaConfigManager::currentPid() const
{
    return readPidFile(m_path + QLatin1String("/vidalia.pid"));
}

bool VidaliaConfigManager::isVidaliaRunning() const
{
    return isProcessRunning(currentPid());
}

bool VidaliaConfigManager::hasCompatibleConfig() const
{
    const QSettings settings(configPath(), QSettings::IniFormat);
    if (settings.status() != QSettings::NoError)
        return false;

    QString authMethod = settings.value(QLatin1String("Tor/AuthenticationMethod"),
                                        QLatin1String("password")).toString();

    if (authMethod == QLatin1String("none") || authMethod == QLatin1String("cookie"))
        return true;

    if (authMethod != QLatin1String("password"))
    {
        qWarning() << "Unrecognized value for Vidalia's Tor/AuthenticationMethod variable:" << authMethod;
        return false;
    }

    if (settings.value(QLatin1String("Tor/UseRandomPassword"), true).toBool())
        return false;

    if (!settings.value(QLatin1String("Tor/ControlPassword")).toString().isEmpty())
        return true;

    return false;
}

void VidaliaConfigManager::getControlInfo(QString *address, quint16 *port, QByteArray *password) const
{
    const QSettings settings(configPath(), QSettings::IniFormat);

    *address = settings.value(QLatin1String("Tor/ControlAddr"), QLatin1String("127.0.0.1")).toString();
    *port = static_cast<quint16>(settings.value(QLatin1String("Tor/ControlPort"), 9051).toUInt());
    if (password)
        *password = settings.value(QLatin1String("Tor/ControlPassword")).toString().toLocal8Bit();
}

bool VidaliaConfigManager::reconfigureControlConfig(QString *errorMessage)
{
    /* Alter Vidalia configuration */
    QSettings settings(configPath(), QSettings::IniFormat);

    if (settings.status() == QSettings::NoError)
    {
        settings.setValue(QLatin1String("Tor/AuthenticationMethod"), QLatin1String("cookie"));
        settings.sync();
    }

    if (settings.status() != QSettings::NoError)
    {
        if (errorMessage)
            *errorMessage = tr("Unable to modify Vidalia configuration (error %1)").arg(settings.status());
        return false;
    }

    Q_ASSERT(hasCompatibleConfig());

    return true;
}
