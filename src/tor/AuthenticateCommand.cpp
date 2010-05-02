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

#include "AuthenticateCommand.h"

using namespace Tor;

AuthenticateCommand::AuthenticateCommand()
	: TorControlCommand("AUTHENTICATE")
{
}

QByteArray AuthenticateCommand::build(const QByteArray &data)
{
	if (data.isNull())
		return QByteArray("AUTHENTICATE\r\n");

	return QByteArray("AUTHENTICATE ") + data.toHex() + "\r\n";
}

void AuthenticateCommand::handleReply(int code, QByteArray &data, bool end)
{
        Q_UNUSED(code);
	if (end)
		statusMessage = data;
}
