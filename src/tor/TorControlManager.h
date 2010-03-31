#ifndef TORCONTROLMANAGER_H
#define TORCONTROLMANAGER_H

#include <QObject>
#include <QHostAddress>

namespace Tor
{

class HiddenService
{
	Q_DISABLE_COPY(HiddenService)

public:
	struct Target
	{
		QHostAddress targetAddress;
		quint16 servicePort, targetPort;
	};

	enum Status
	{
		Offline,
		Published,
		Online
	};

	const QString dataPath;

	HiddenService(const QString &dataPath);

	Status status() const { return pStatus; }

	const QList<Target> &targets() const { return pTargets; }
	void addTarget(const Target &target);
	void addTarget(quint16 servicePort, QHostAddress targetAddress, quint16 targetPort);

private:
	QList<Target> pTargets;
	Status pStatus;
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
	bool isConnected() const { return status() == Connected; }
	void connect(const QHostAddress &address, quint16 port);

	/* Hidden Services */
	const QList<HiddenService*> &hiddenServices() const { return pServices; }
	void addHiddenService(HiddenService *service);

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
	QList<HiddenService*> pServices;
	QFlags<AuthMethod> pAuthMethods;
	Status pStatus;

	void setStatus(Status status);

	void authenticate();
	void getSocksInfo();
	void publishServices();
};

}

extern Tor::TorControlManager *torManager;

#endif // TORCONTROLMANAGER_H
