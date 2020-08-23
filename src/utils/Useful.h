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

#ifndef UTILS_USEFUL_H
#define UTILS_USEFUL_H

#include <QtGlobal>
#include <QDebug>

/* Print a warning for bug conditions, and assert on a debug build.
 *
 * This should be used in place of Q_ASSERT for bug conditions, along
 * with a proper error case for release-mode builds. For example:
 *
 * if (!connection || !user) {
 *     BUG() << "Request" << request << "should have a connection and user";
 *     return false;
 * }
 *
 * Do not confuse bugs with actual error cases; BUG() should never be
 * triggered unless the code or logic is wrong.
 */
#if !defined(QT_NO_DEBUG) || defined(QT_FORCE_ASSERTS)
# define BUG() Explode(__FILE__,__LINE__), qWarning() << "BUG:"
namespace {
class Explode
{
public:
    const char *file;
    int line;
    Explode(const char *file, int line) : file(file), line(line) { }
    ~Explode() {
        qt_assert("something broke!", file, line);
    }
};
}
#else
# define BUG() qWarning() << "BUG:"
#endif

#endif

