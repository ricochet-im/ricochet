#ifndef TORCONTROLSOCKET_H
#define TORCONTROLSOCKET_H

#include <QTcpSocket>

class TorControlSocket : public QTcpSocket
{
Q_OBJECT
public:
    explicit TorControlSocket(QObject *parent = 0);

private slots:
	void process();
};

#endif // TORCONTROLSOCKET_H
