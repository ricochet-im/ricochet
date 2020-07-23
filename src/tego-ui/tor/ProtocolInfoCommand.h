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

#ifndef PROTOCOLINFOCOMMAND_H
#define PROTOCOLINFOCOMMAND_H

#include "TorControlCommand.h"
#include <QFlags>

namespace Tor
{

class TorControl;

class ProtocolInfoCommand : public TorControlCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(ProtocolInfoCommand)

public:
    enum AuthMethod
    {
        AuthUnknown = 0,
        AuthNull = 0x1,
        AuthHashedPassword = 0x2,
        AuthCookie = 0x4
    };
    Q_DECLARE_FLAGS(AuthMethods, AuthMethod)

    ProtocolInfoCommand(TorControl *manager);
    QByteArray build();

    AuthMethods authMethods() const { return m_authMethods; }
    QString torVersion() const { return m_torVersion; }
    QString cookieFile() const { return m_cookieFile; }

protected:
    virtual void onReply(int statusCode, const QByteArray &data);

private:
    TorControl *manager;
    AuthMethods m_authMethods;
    QString m_torVersion;
    QString m_cookieFile;
};

}

#endif // PROTOCOLINFOCOMMAND_H
