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
