/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
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

#include "ProtocolInfoCommand.h"
#include "TorControl.h"
#include "utils/StringUtil.h"
#include <QList>

using namespace Tor;

ProtocolInfoCommand::ProtocolInfoCommand(TorControl *m)
    : manager(m)
{
}

QByteArray ProtocolInfoCommand::build()
{
    return QByteArray("PROTOCOLINFO 1\r\n");
}

void ProtocolInfoCommand::onReply(int statusCode, const QByteArray &data)
{
    TorControlCommand::onReply(statusCode, data);
    if (statusCode != 250)
        return;

    if (data.startsWith("AUTH "))
    {
        QList<QByteArray> tokens = splitQuotedStrings(data.mid(5), ' ');

        foreach (QByteArray token, tokens)
        {
            if (token.startsWith("METHODS="))
            {
                QList<QByteArray> textMethods = unquotedString(token.mid(8)).split(',');
                for (QList<QByteArray>::Iterator it = textMethods.begin(); it != textMethods.end(); ++it)
                {
                    if (*it == "NULL")
                        m_authMethods |= AuthNull;
                    else if (*it == "HASHEDPASSWORD")
                        m_authMethods |= AuthHashedPassword;
                    else if (*it == "COOKIE")
                        m_authMethods |= AuthCookie;
                }
            }
            else if (token.startsWith("COOKIEFILE="))
            {
                m_cookieFile = QString::fromLatin1(unquotedString(token.mid(11)));
            }
        }
    }
    else if (data.startsWith("VERSION Tor="))
    {
        m_torVersion = QString::fromLatin1(unquotedString(data.mid(12, data.indexOf(' ', 12))));
    }
}
