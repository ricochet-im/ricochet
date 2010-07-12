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

#ifndef TORCONTROLSOCKET_H
#define TORCONTROLSOCKET_H

#include <QTcpSocket>
#include <QQueue>

namespace Tor
{

class TorControlCommand;

class TorControlSocket : public QTcpSocket
{
Q_OBJECT
public:
    explicit TorControlSocket(QObject *parent = 0);

    void sendCommand(const QByteArray &data) { sendCommand(0, data); }
    void sendCommand(TorControlCommand *command, const QByteArray &data);

signals:
    void commandFinished(TorControlCommand *command);
    void controlError(const QString &message);

private slots:
    void process();

private:
    QQueue<TorControlCommand*> commandQueue;
};

}

#endif // TORCONTROLSOCKET_H
