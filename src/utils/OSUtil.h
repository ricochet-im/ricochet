/* Torsion - http://torsionim.org/
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

#ifndef OSUTIL_H
#define OSUTIL_H

#include <QtGlobal>
#include <QString>

/* Return the PID written to the given file, or -1 */
qint64 readPidFile(const QString &path);
bool isProcessRunning(qint64 pid);
/* Attempt to kill a process, gracefully or forcefully. Process may not exit immediately. */
bool killProcess(qint64 pid, bool forceful = false);

/* Acquire a lock related to the given file path; due to platform limitations, a separate
 * lock file may be used instead of locking the given file directly. */
class FileLock
{
public:
    FileLock(const QString &path = QString());
    ~FileLock();

    QString path() const { return m_path; }
    void setPath(const QString &path);

    /* -1 indicates error, 0 indicates that the file has been locked by another process, 1 is success */
    int acquire();
    void release();

    /* True if the file is locked *by this process* */
    bool isLocked() const { return m_locked; }

private:
    QString m_path;
    union
    {
        void *handle;
        int fd;
    };
    bool m_locked;

    QString lockFile() const;
};

#endif // OSUTIL_H
