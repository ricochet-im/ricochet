#ifndef TORCONTROLMANAGER_H
#define TORCONTROLMANAGER_H

#include <QObject>

class TorControlManager : public QObject
{
Q_OBJECT
public:
    explicit TorControlManager(QObject *parent = 0);

public slots:
	void connect();

private slots:
	void authenticate();

private:
	class TorControlSocket *socket;
};

#endif // TORCONTROLMANAGER_H
