/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
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
 * along with TorIM. If not, see http://www.gnu.org/licenses/
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
        return QApplication::translate("timeDifferenceString", "%n minute(s) ago", "", QApplication::CodecForTr,
                                       qRound((qreal)secs / 60));
    else if (secs < 86400)
        return QApplication::translate("timeDifferenceString", "%n hour(s) ago", "", QApplication::CodecForTr,
                                       qRound((qreal)secs / 3600));
    else if (secs < 604800)
        return QApplication::translate("timeDifferenceString", "%n day(s) ago", "", QApplication::CodecForTr,
                                       qRound((qreal)secs / 86400));
    else if (secs < 2592000)
        return QApplication::translate("timeDifferenceString", "%n week(s) ago", "", QApplication::CodecForTr,
                                       qRound((qreal)secs / 604800));
    else if (secs < 31536000)
        return QApplication::translate("timeDifferenceString", "%n month(s) ago", "", QApplication::CodecForTr,
                                       qRound((qreal)secs / 2592000));
    else
        return QApplication::translate("timeDifferenceString", "%n year(s) ago", "", QApplication::CodecForTr,
                                       qRound((qreal)secs / 31536000));
}
