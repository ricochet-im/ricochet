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

#ifndef SETCONFCOMMAND_H
#define SETCONFCOMMAND_H

#include "TorControlCommand.h"
#include <QList>
#include <QPair>

namespace Tor
{

class SetConfCommand : public TorControlCommand
{
    Q_OBJECT
    Q_DISABLE_COPY(SetConfCommand)

public:
    QByteArray statusMessage;

    SetConfCommand();

    QByteArray build(const QByteArray &key, const QByteArray &value);
    QByteArray build(const QList<QPair<QByteArray,QByteArray> > &data);

signals:
    void setConfSucceeded();
    void setConfFailed(int code);

protected:
    virtual void handleReply(int code, QByteArray &data, bool end);
};

}

#endif // SETCONFCOMMAND_H
