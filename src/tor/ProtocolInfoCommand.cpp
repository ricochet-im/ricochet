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

#include "ProtocolInfoCommand.h"
#include "TorControlManager.h"
#include "utils/StringUtil.h"
#include <QList>

using namespace Tor;

ProtocolInfoCommand::ProtocolInfoCommand(TorControlManager *m)
    : TorControlCommand("PROTOCOLINFO"), manager(m)
{
}

QByteArray ProtocolInfoCommand::build()
{
    return QByteArray("PROTOCOLINFO 1\r\n");
}

void ProtocolInfoCommand::handleReply(int code, QByteArray &data, bool end)
{
    Q_UNUSED(end);

    if (code != 250)
        return;

    if (data.startsWith("AUTH METHODS="))
    {
        int methodsEnd = data.indexOf(' ', 13);

        QList<QByteArray> textMethods = data.mid(13, methodsEnd).split(',');
        for (QList<QByteArray>::Iterator it = textMethods.begin(); it != textMethods.end(); ++it)
        {
            if (*it == "NULL")
                m_authMethods |= AuthNull;
            else if (*it == "HASHEDPASSWORD")
                m_authMethods |= AuthHashedPassword;
            else if (*it == "COOKIE")
                m_authMethods |= AuthCookie;
        }

        if (data.mid(methodsEnd+1).startsWith("COOKIEFILE="))
            m_cookieFile = QString::fromLatin1(unquotedString(data.mid(methodsEnd+12,
                                                                       data.indexOf(' ', methodsEnd+12))));
    }
    else if (data.startsWith("VERSION Tor="))
    {
        m_torVersion = QString::fromLatin1(unquotedString(data.mid(12, data.indexOf(' ', 12))));
    }
}
