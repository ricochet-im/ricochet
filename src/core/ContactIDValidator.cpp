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

#include "ContactIDValidator.h"
#include <QRegExp>

ContactIDValidator::ContactIDValidator(QObject *parent)
    : QValidator(parent)
{
}

QValidator::State ContactIDValidator::validate(QString &text, int &pos) const
{
    Q_UNUSED(pos);

    /* [a-z2-7]{16}@Torsion */
    for (int i = 0; i < qMin(text.size(), 16); ++i)
    {
        char c = text[i].toLatin1();
        if (!((c >= 'a' && c <= 'z') || (c >= '2' && c <= '7')))
            return QValidator::Invalid;
    }

    if (text.size() < 16)
        return QValidator::Intermediate;
    else if (text.size() > 24)
        return QValidator::Invalid;

    QString suffix = QLatin1String("@Torsion");
    suffix.truncate(text.size() - 16);

    if (QString::compare(text.mid(16), suffix, Qt::CaseInsensitive) != 0)
        return QValidator::Invalid;

    if (suffix.size() != 8)
        return QValidator::Intermediate;

    return QValidator::Acceptable;
}

bool ContactIDValidator::isValidID(const QString &text)
{
    QRegExp regex(QLatin1String("^[a-z2-7]{16}@Torsion$"), Qt::CaseInsensitive);
    return regex.exactMatch(text);
}

QString ContactIDValidator::hostnameFromID(const QString &ID)
{
    if (!isValidID(ID))
        return QString();

    return ID.mid(0, 16) + QLatin1String(".onion");
}

QString ContactIDValidator::idFromHostname(const QString &hostname)
{
    QString re = hostname;

    if (re.size() != 16)
    {
        if (re.size() == 22 && re.endsWith(QLatin1String(".onion")))
            re.chop(6);
        else
            return QString();
    }

    re += QLatin1String("@Torsion");

    if (!isValidID(re))
        return QString();
    return re;
}
