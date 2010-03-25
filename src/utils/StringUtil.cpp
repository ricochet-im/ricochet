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
