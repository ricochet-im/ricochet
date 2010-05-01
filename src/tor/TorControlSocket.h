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

	void sendCommand(const QByteArray &data);
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
