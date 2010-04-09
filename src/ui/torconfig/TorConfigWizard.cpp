#include "TorConfigWizard.h"
#include "IntroPage.h"
#include "ManualConfigPage.h"

using namespace TorConfig;

TorConfigWizard::TorConfigWizard(QWidget *parent)
	: QWizard(parent)
{
	setWindowTitle(tr("TorIM - Configure Tor"));
	setFixedSize(600, 450);

	addPage(new IntroPage);
	addPage(new ManualConfigPage);
}
