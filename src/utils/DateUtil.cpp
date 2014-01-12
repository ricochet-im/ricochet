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

#include "DateUtil.h"
#include <QDateTime>
#include <QApplication>

QString timeDifferenceString(const QDateTime &from, const QDateTime &to)
{
    if (from.isNull())
        return QApplication::translate("timeDifferenceString", "Never");

    int secs = from.secsTo(to);

    if (secs < 0)
        return QApplication::translate("timeDifferenceString", "In the future");
    else if (secs < 60)
        return QApplication::translate("timeDifferenceString", "Moments ago");
    else if (secs < 3600)
        return QApplication::translate("timeDifferenceString", "%n minute(s) ago", "", qRound((qreal)secs / 60));
    else if (secs < 86400)
        return QApplication::translate("timeDifferenceString", "%n hour(s) ago", "", qRound((qreal)secs / 3600));
    else if (secs < 604800)
        return QApplication::translate("timeDifferenceString", "%n day(s) ago", "", qRound((qreal)secs / 86400));
    else if (secs < 2592000)
        return QApplication::translate("timeDifferenceString", "%n week(s) ago", "", qRound((qreal)secs / 604800));
    else if (secs < 31536000)
        return QApplication::translate("timeDifferenceString", "%n month(s) ago", "", qRound((qreal)secs / 2592000));
    else
        return QApplication::translate("timeDifferenceString", "%n year(s) ago", "", qRound((qreal)secs / 31536000));
}
