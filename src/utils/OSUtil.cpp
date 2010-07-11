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

#include "OSUtil.h"
#include <QString>
#include <QFile>

#ifdef Q_OS_WIN
#include <Windows.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#endif

qint64 readPidFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return -1;

    bool ok = false;
    qint64 re = static_cast<qint64>(file.read(12).trimmed().toULongLong(&ok));
    if (!ok)
        re = -1;

    return re;
}

bool isProcessRunning(qint64 pid)
{
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

#ifdef Q_OS_WIN
bool killProcess(qint64 pid, bool forceful)
{
    Q_UNUSED(forceful);

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, static_cast<DWORD>(pid));
    if (!hProcess)
        return false;

    bool ok = TerminateProcess(hProcess, 0);
    CloseHandle(hProcess);

    return ok;
}
#else
bool killProcess(qint64 pid, bool forceful)
{
    int r = kill(static_cast<pid_t>(pid), forceful ? SIGKILL : SIGTERM);
    return (r >= 0) || (errno == ESRCH);
}
#endif
