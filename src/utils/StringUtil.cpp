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

QByteArray unquotedString(const QByteArray &string, int *pos)
{
	if (!string.startsWith('"'))
	{
		if (pos)
			*pos = string.size();
		return string;
	}

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
			if (pos)
				*pos = i+1;
			return out;
		default:
			out.append(string[i]);
		}
	}

	if (pos)
		*pos = -1;
	return out;
}
