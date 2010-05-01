#include "main.h"
#include "TorConfigWizard.h"
#include "IntroPage.h"
#include "ManualConfigPage.h"
#include <QMessageBox>

using namespace TorConfig;

TorConfigWizard::TorConfigWizard(QWidget *parent)
	: QWizard(parent)
{
	setWindowTitle(tr("TorIM - Configure Tor"));
	setFixedSize(550, 450);

	addPage(new IntroPage);
	addPage(new QWizardPage);
	addPage(new ManualConfigPage);
}

void TorConfigWizard::accept()
{
	QString controlIp = field(QString("controlIp")).toString();
	quint16 controlPort = (quint16) field(QString("controlPort")).toUInt();

	if (controlIp.isEmpty() || controlPort < 1)
	{
		QMessageBox::critical(this, tr("Error"), tr("The wizard is incomplete; please go back and ensure all required"
													"fields are filled"));
		return;
	}

	config->setValue(QString("tor/controlIp"), field(QString("controlIp")));
	config->setValue(QString("tor/controlPort"), field(QString("controlPort")));

	QString authPassword = field(QString("controlPassword")).toString();
	if (!authPassword.isEmpty())
		config->setValue(QString("tor/authPassword"), authPassword);
	else
		config->remove(QString("tor/authPassword"));

	QWizard::accept();
}
