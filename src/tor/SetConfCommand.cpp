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

#include "SetConfCommand.h"
#include "utils/StringUtil.h"

using namespace Tor;

SetConfCommand::SetConfCommand()
    : TorControlCommand("SETCONF")
{
}

QByteArray SetConfCommand::build(const QByteArray &key, const QByteArray &value)
{
    return QByteArray("SETCONF ") + key + "=" + quotedString(value) + "\r\n";
}

QByteArray SetConfCommand::build(const QList<QPair<QByteArray, QByteArray> > &data)
{
    if (data.isEmpty())
        return QByteArray();

    QByteArray out("SETCONF");
    for (int i = 0; i < data.size(); ++i)
    {
        out.append(' ');
        out.append(data[i].first);
        out.append('=');
        out.append(quotedString(data[i].second));
    }

    out.append("\r\n");
    return out;
}

void SetConfCommand::handleReply(int code, QByteArray &data, bool end)
{
    Q_UNUSED(code);

    if (end)
    {
        statusMessage = data;

        if (code == 250)
            emit setConfSucceeded();
        else
            emit setConfFailed(code);
    }
}
