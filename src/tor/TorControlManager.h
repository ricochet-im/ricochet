#ifndef TORCONTROLMANAGER_H
#define TORCONTROLMANAGER_H

#include <QObject>
#include <QHostAddress>

namespace Tor
{

class HiddenService
{
	friend class TorControlManager;

public:
	QString dataPath;
	QHostAddress targetAddress;
	quint16 servicePort, targetPort;

private:
	HiddenService(const QString &path, const QHostAddress &address, quint16 port);
};

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

	enum Status
	{
		Error = -1,
		NotConnected,
		Connecting,
		Authenticating,
		Connected
	};

    explicit TorControlManager(QObject *parent = 0);

	/* Information */
	Status status() const { return pStatus; }
	QString torVersion() const { return pTorVersion; }

	/* Authentication */
	QFlags<AuthMethod> authMethods() const { return pAuthMethods; }
	void setAuthPassword(const QByteArray &password);

	/* Connection */
	bool isConnected() const;
	void connect(const QHostAddress &address, quint16 port);

	/* Hidden Services */
	HiddenService *createHiddenService(const QString &path, const QHostAddress &address, quint16 port);
	const QList<HiddenService*> &hiddenServices() const;

signals:
	void statusChanged(Status newStatus, Status oldStatus);
	void connected();
	void disconnected();

private slots:
	void socketConnected();

	void commandFinished(class TorControlCommand *command);

private:
	class TorControlSocket *socket;
	QString pTorVersion;
	QByteArray pAuthPassword;
	QList<HiddenService*> pHiddenServices;
	QFlags<AuthMethod> pAuthMethods;
	Status pStatus;

	void setStatus(Status status);

	void authenticate();
};

}

#endif // TORCONTROLMANAGER_H
