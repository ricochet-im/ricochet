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

#ifndef TORCONTROLCOMMAND_H
#define TORCONTROLCOMMAND_H

#include <QObject>
#include <QByteArray>

namespace Tor
{

class TorControlCommand : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(TorControlCommand)

    friend class TorControlSocket;

public:
    const char * const keyword;

    TorControlCommand(const char *keyword);

    int statusCode() const { return pStatusCode; }

signals:
    void replyFinished();

protected:
    virtual void handleReply(int code, QByteArray &data, bool end) = 0;

private:
    int pStatusCode;

    void inputReply(int code, QByteArray &data, bool end);
};

}

#endif // TORCONTROLCOMMAND_H
