#include "TorControlSocket.h"
#include <QDebug>

TorControlSocket::TorControlSocket(QObject *parent) :
    QTcpSocket(parent)
{
	connect(this, SIGNAL(readyRead()), this, SLOT(process()));
}

void TorControlSocket::process()
{
	QByteArray data = read(2048);
	qDebug() << "Read: " << QString::fromLatin1(data);
}
