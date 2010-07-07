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

#include "GetConfCommand.h"
#include "utils/StringUtil.h"

using namespace Tor;

GetConfCommand::GetConfCommand()
    : TorControlCommand("GETCONF")
{
}

QByteArray GetConfCommand::build(const QByteArray &key)
{
    return QByteArray("GETCONF ") + key + "\r\n";
}

QByteArray GetConfCommand::build(const QList<QByteArray> &keys)
{
    QByteArray out("GETCONF");
    for (QList<QByteArray>::ConstIterator it = keys.begin(); it != keys.end(); ++it)
    {
        out.append(' ');
        out.append(*it);
    }

    out.append("\r\n");
    return out;
}

void GetConfCommand::handleReply(int code, QByteArray &data, bool end)
{
        Q_UNUSED(end);

    if (code != 250)
        return;

    int kep = data.indexOf('=');
    pResults.insertMulti(data.mid(0, kep), (kep >= 0) ? data.mid(kep+1) : QByteArray());
}

bool GetConfCommand::get(const QByteArray &key, QByteArray &value) const
{
    QMultiHash<QByteArray,QByteArray>::ConstIterator it = pResults.find(key);
    if (it == pResults.end())
        return false;

    value = *it;
    return true;
}

QList<QByteArray> GetConfCommand::getList(const QByteArray &key) const
{
    /* QHash returns values from the most recent to the least recent, but Tor sends its values
     * from first to last, and order may be relevant. So, reverse the list */

    QList<QByteArray> values = pResults.values(key);
    QList<QByteArray> out;

    QList<QByteArray>::Iterator it = values.end();
    while (it != values.begin())
    {
        --it;
        out.append(*it);
    }

    return out;
}
