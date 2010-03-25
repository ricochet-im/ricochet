#include "TorControlSocket.h"
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

	qDebug() << "torctrl: Sent " << data;
}

void TorControlSocket::process()
{
	QByteArray data = read(2048);
	qDebug() << "Read: " << QString::fromLatin1(data);
}
