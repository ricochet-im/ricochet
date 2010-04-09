#include "HiddenService.h"

using namespace Tor;

HiddenService::HiddenService(const QString &p)
	: dataPath(p), pStatus(Offline)
{
}

void HiddenService::addTarget(const Target &target)
{
	pTargets.append(target);
}

void HiddenService::addTarget(quint16 servicePort, QHostAddress targetAddress, quint16 targetPort)
{
	Target t = { targetAddress, servicePort, targetPort };
	pTargets.append(t);
}
