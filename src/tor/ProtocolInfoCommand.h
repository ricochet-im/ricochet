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

#ifndef PROTOCOLINFOCOMMAND_H
#define PROTOCOLINFOCOMMAND_H

#include "TorControlCommand.h"
#include <QFlags>

namespace Tor
{

class TorControlManager;

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

    ProtocolInfoCommand(TorControlManager *manager);
    QByteArray build();

    AuthMethods authMethods() const { return m_authMethods; }
    QString torVersion() const { return m_torVersion; }
    QString cookieFile() const { return m_cookieFile; }

protected:
    virtual void handleReply(int code, QByteArray &data, bool end);

private:
    TorControlManager *manager;
    AuthMethods m_authMethods;
    QString m_torVersion;
    QString m_cookieFile;
};

}

#endif // PROTOCOLINFOCOMMAND_H
