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
