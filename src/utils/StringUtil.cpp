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

#include "StringUtil.h"

QByteArray quotedString(const QByteArray &string)
{
    QByteArray out;
    out.reserve(string.size() * 2);

    out.append('"');

    for (int i = 0; i < string.size(); ++i)
    {
        switch (string[i])
        {
        case '"':
            out.append("\\\"");
            break;
        case '\\':
            out.append("\\\\");
            break;
        default:
            out.append(string[i]);
            break;
        }
    }

    out.append('"');
    return out;
}

QByteArray unquotedString(const QByteArray &string)
{
    if (string[0] != '"')
        return string;

    QByteArray out;
    out.reserve(string.size() - 2);

    for (int i = 1; i < string.size(); ++i)
    {
        switch (string[i])
        {
        case '\\':
            if (++i < string.size())
                out.append(string[i]);
            break;
        case '"':
            return out;
        default:
            out.append(string[i]);
        }
    }

    return out;
}

QList<QByteArray> splitQuotedStrings(const QByteArray &input, char separator)
{
    QList<QByteArray> out;
    bool inquote = false;
    int start = 0;

    for (int i = 0; i < input.size(); ++i)
    {
        switch (input[i])
        {
        case '"':
            inquote = !inquote;
            break;
        case '\\':
            if (inquote)
                ++i;
            break;
        }

        if (!inquote && input[i] == separator)
        {
            out.append(input.mid(start, i));
            start = i+1;
        }
    }

    if (start < input.size())
        out.append(input.mid(start));

    return out;
}
