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

#include "OSUtil.h"
#include <QString>
#include <QFile>
#include <QDir>

#ifdef Q_OS_WIN
#include <Windows.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
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

FileLock::FileLock(const QString &p)
    : m_locked(false)
{
    setPath(p);
}

FileLock::~FileLock()
{
    release();
}

QString FileLock::lockFile() const
{
    return QDir::toNativeSeparators(m_path + QLatin1String(".lock"));
}

void FileLock::setPath(const QString &p)
{
    release();
    m_path = p;
}

#ifdef Q_OS_WIN
int FileLock::acquire()
{
    if (isLocked())
        return 1;

    QDir parentDir = QFileInfo(lockFile()).dir();
    if (!parentDir.exists())
    {
        if (!parentDir.mkpath(parentDir.absolutePath()))
            qWarning("Error when creating directory for lock file");
    }

    handle = CreateFile(reinterpret_cast<LPCWSTR>(lockFile().utf16()), GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_ALWAYS,
                        FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_DELETE_ON_CLOSE, 0);
    if (handle == INVALID_HANDLE_VALUE)
        return -1;

    BOOL ok = LockFile(handle, 0, 0, 1, 0);
    if (!ok)
    {
        DWORD error = GetLastError();
        CloseHandle(handle);
        if (error == ERROR_LOCK_VIOLATION)
            return 0;
        return -1;
    }

    m_locked = true;
    return 1;
}

void FileLock::release()
{
    if (!isLocked())
        return;

    UnlockFile(handle, 0, 0, 1, 0);
    CloseHandle(handle);

    m_locked = false;
}
#else
int FileLock::acquire()
{
    if (isLocked())
        return 1;

    QDir parentDir = QFileInfo(lockFile()).dir();
    if (!parentDir.exists())
    {
        if (!parentDir.mkpath(parentDir.absolutePath()))
            qWarning("Error when creating directory for lock file");
    }

    fd = open(lockFile().toLocal8Bit().constData(), O_RDWR | O_CREAT | O_TRUNC, 0600);
    if (fd < 0)
        return -1;

    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(fd, F_SETLK, &lock) < 0)
    {
        int err = errno;
        close(fd);
        if (err != EACCES && err != EAGAIN)
            return -1;
        return 0;
    }

    m_locked = true;
    return 1;
}

void FileLock::release()
{
    if (!isLocked())
        return;

    close(fd);

    /* Attempt to delete the lock file; it doesn't really matter if this fails */
    unlink(lockFile().toLocal8Bit().constData());
}
#endif
