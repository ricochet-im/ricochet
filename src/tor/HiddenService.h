#ifndef HIDDENSERVICE_H
#define HIDDENSERVICE_H

#include <QObject>
#include <QHostAddress>
#include <QList>

namespace Tor
{

class HiddenService : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(HiddenService)

	friend class TorControlManager;

public:
	struct Target
	{
		QHostAddress targetAddress;
		quint16 servicePort, targetPort;
	};

	enum Status
	{
		Offline, /* Not published */
		Published, /* Published, but not confirmed to be accessible */
		Online /* Published and accessible */
	};

	const QString dataPath;

	HiddenService(const QString &dataPath);

	Status status() const { return pStatus; }

	const QString &hostname() const { return pHostname; }

	const QList<Target> &targets() const { return pTargets; }
	void addTarget(const Target &target);
	void addTarget(quint16 servicePort, QHostAddress targetAddress, quint16 targetPort);

public slots:
	void startSelfTest();

signals:
	void statusChanged(int newStatus, int oldStatus);
	void serviceOnline();

private slots:
	void servicePublished();
	void selfTestSucceeded();
	void selfTestFailed();

private:
	QList<Target> pTargets;
	QString pHostname;
	class TorServiceTest *selfTest;
	Status pStatus;

	void setStatus(Status newStatus);
	void readHostname();
};

}

#endif // HIDDENSERVICE_H
