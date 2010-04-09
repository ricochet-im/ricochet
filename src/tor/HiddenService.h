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

}

#endif // HIDDENSERVICE_H
