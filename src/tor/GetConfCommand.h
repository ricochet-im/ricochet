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

#ifndef GETCONFCOMMAND_H
#define GETCONFCOMMAND_H

#include "TorControlCommand.h"
#include <QList>
#include <QMultiHash>

namespace Tor
{

class GetConfCommand : public TorControlCommand
{
	Q_OBJECT
	Q_DISABLE_COPY(GetConfCommand)

public:
    GetConfCommand();

	QByteArray build(const QByteArray &key);
	QByteArray build(const QList<QByteArray> &keys);

	const QMultiHash<QByteArray,QByteArray> &results() const { return pResults; }
	bool get(const QByteArray &key, QByteArray &value) const;
	QList<QByteArray> getList(const QByteArray &key) const;

protected:
	virtual void handleReply(int code, QByteArray &data, bool end);

private:
	QMultiHash<QByteArray,QByteArray> pResults;
};

}

#endif // GETCONFCOMMAND_H
