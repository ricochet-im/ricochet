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

#include "SetConfCommand.h"
#include "utils/StringUtil.h"

#include "globals.hpp"
using tego::g_globals;

using namespace Tor;

SetConfCommand::SetConfCommand()
    : m_resetMode(false)
{
}

void SetConfCommand::setResetMode(bool enabled)
{
    m_resetMode = enabled;
}

bool SetConfCommand::isSuccessful() const
{
    return statusCode() == 250;
}

QByteArray SetConfCommand::build(const QByteArray &key, const QByteArray &value)
{
    return build(QList<QPair<QByteArray, QByteArray> >() << qMakePair(key, value));
}

QByteArray SetConfCommand::build(const QVariantMap &data)
{
    QList<QPair<QByteArray, QByteArray> > out;

    for (QVariantMap::ConstIterator it = data.begin(); it != data.end(); it++) {
        QByteArray key = it.key().toLatin1();

        if (static_cast<QMetaType::Type>(it.value().type()) == QMetaType::QVariantList) {
            QVariantList values = it.value().value<QVariantList>();
            foreach (const QVariant &value, values)
                out.append(qMakePair(key, value.toString().toLatin1()));
        } else {
            out.append(qMakePair(key, it.value().toString().toLatin1()));
        }
    }

    return build(out);
}

QByteArray SetConfCommand::build(const QList<QPair<QByteArray, QByteArray> > &data)
{
    QByteArray out(m_resetMode ? "RESETCONF" : "SETCONF");

    for (int i = 0; i < data.size(); i++) {
        out += " " + data[i].first;
        if (!data[i].second.isEmpty())
            out += "=" + quotedString(data[i].second);
    }

    out.append("\r\n");
    return out;
}

void SetConfCommand::onReply(int statusCode, const QByteArray &data)
{
    TorControlCommand::onReply(statusCode, data);
    if (statusCode != 250)
        m_errorMessage = QString::fromLatin1(data);
}

void SetConfCommand::onFinished(int statusCode)
{
    TorControlCommand::onFinished(statusCode);
    if (isSuccessful())
        emit setConfSucceeded();
    else
        emit setConfFailed(statusCode);

    g_globals.context->callback_registry_.emit_update_tor_daemon_config_succeeded(isSuccessful() ? TEGO_TRUE : TEGO_FALSE);
}

