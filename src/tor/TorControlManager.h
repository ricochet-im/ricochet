#ifndef TORCONTROLMANAGER_H
#define TORCONTROLMANAGER_H

#include <QObject>
#include <QHostAddress>

namespace Tor
{

class TorControlManager : public QObject
{
	Q_OBJECT
	friend class ProtocolInfoCommand;

public:
	enum AuthMethod
	{
		AuthUnknown = 0,
		AuthNull = 0x1,
		AuthHashedPassword = 0x2,
		AuthCookie = 0x4
	};

    explicit TorControlManager(QObject *parent = 0);

	QFlags<AuthMethod> authMethods() const { return pAuthMethods; }
	QByteArray torVersion() const { return pTorVersion; }

	void setAuthPassword(const QByteArray &password);

	void connect(const QHostAddress &address, quint16 port);
	void createHiddenService(const QString &path, const QHostAddress &address, quint16 port);

private slots:
	void queryInfo();

	void commandFinished(class TorControlCommand *command);

private:
	class TorControlSocket *socket;
	QFlags<AuthMethod> pAuthMethods;
	QByteArray pTorVersion, pAuthPassword;

	void authenticate();
};

}

#endif // TORCONTROLMANAGER_H
