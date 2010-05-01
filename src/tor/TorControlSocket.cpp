#include "TorControlSocket.h"
#include "TorControlCommand.h"
#include <QDebug>

using namespace Tor;

TorControlSocket::TorControlSocket(QObject *parent) :
    QTcpSocket(parent)
{
	connect(this, SIGNAL(readyRead()), this, SLOT(process()));
}

void TorControlSocket::sendCommand(TorControlCommand *command, const QByteArray &data)
{
	commandQueue.append(command);

	Q_ASSERT(data.endsWith("\r\n"));
	write(data);

	qDebug() << "torctrl: Sent" << data;
}

void TorControlSocket::process()
{
	for (;;)
	{
		if (!canReadLine())
			return;

		QByteArray line = readLine(5120);

		if (line.size() < 4 || !line.endsWith("\r\n"))
		{
			controlError(tr("Invalid control message syntax (may not be a Tor control port)"));
			return;
		}

		if (line[3] == '+')
		{
			controlError(tr("BUG: Data replies are not supported"));
			return;
		}

		int code = line.left(3).toInt();
		bool end = (line[3] == ' ');

		if (!end && line[3] != '-')
		{
			controlError(tr("Invalid or unrecognized syntax (may not be a Tor control port)"));
			return;
		}

		if (commandQueue.isEmpty())
		{
			qWarning("torctrl: Received unexpected data");
			return;
		}

		TorControlCommand *command = commandQueue.first();

		qDebug() << "torctrl: Received" << (end ? "final" : "intermediate") << "reply for"
				<< (command ? command->keyword : "???") << "-" << code << line.mid(4, line.size() - 6);

		if (command)
			command->inputReply(code, line.mid(4, line.size() - 6), end);

		if (end)
		{
			commandQueue.takeFirst();

			emit commandFinished(command);
			command->deleteLater();
		}
	}
}
