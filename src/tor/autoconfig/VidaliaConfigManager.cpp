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

#include "VidaliaConfigManager.h"
#include <QFile>
#include <QDir>
#include <QLibrary>
#include <QSettings>
#include <QtDebug>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <ShlObj.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
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
    QFile pidFile(m_path + QLatin1String("/vidalia.pid"));
    if (!pidFile.open(QIODevice::ReadOnly))
        return -1;

    bool ok = false;
    qint64 re = static_cast<qint64>(pidFile.readAll().toULongLong(&ok));
    if (!ok)
        re = -1;

    return re;
}

bool VidaliaConfigManager::isVidaliaRunning() const
{
    qint64 pid = currentPid();
    if (pid < 0)
        return false;

#ifdef Q_OS_WIN
    DWORD access;
    if (QSysInfo::windowsVersion() & QSysInfo::WV_NT_based && QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA)
        access = PROCESS_QUERY_LIMITED_INFORMATION;
    else
        access = PROCESS_QUERY_INFORMATION;

    HANDLE hProcess = OpenProcess(access, FALSE, static_cast<DWORD>(pid));
    if (!hProcess)
        return false;

    DWORD exitCode;
    if (!GetExitCodeProcess(hProcess, &exitCode))
        exitCode = 0;

    CloseHandle(hProcess);

    return (exitCode == STILL_ACTIVE);
#else
    if (kill(static_cast<pid_t>(pid), 0) < 0)
        return (errno != ESRCH);
    return true;
#endif
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
