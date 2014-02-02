/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
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

#include "GetConfCommand.h"
#include "utils/StringUtil.h"

using namespace Tor;

GetConfCommand::GetConfCommand(const char *type)
    : TorControlCommand(type)
{
    Q_ASSERT((QLatin1String(type) == QLatin1String("GETINFO")) || (QLatin1String(type) == QLatin1String("GETCONF")));
}

QByteArray GetConfCommand::build(const QByteArray &key)
{
    return QByteArray(keyword) + " " + key + "\r\n";
}

QByteArray GetConfCommand::build(const QList<QByteArray> &keys)
{
    QByteArray out(keyword);
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
    QString key = QString::fromLatin1(data.mid(0, kep));
    QVariant value;
    if (kep >= 0)
        value = QString::fromLatin1(unquotedString(data.mid(kep + 1)));

    QVariantMap::iterator it = m_results.find(key);
    if (it != m_results.end()) {
        // Make a list of values
        QVariantList results = it->toList();
        if (results.isEmpty())
            results.append(*it);
        results.append(value);
        *it = QVariant(results);
    } else {
        m_results.insert(key, value);
    }
}

QVariant GetConfCommand::get(const QByteArray &key) const
{
    return m_results.value(QString::fromLatin1(key));
}

