#include "ManualConfigPage.h"
#include "TorConnTestWidget.h"
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegExpValidator>
#include <QIntValidator>
#include <QPushButton>
#include <QVariant>

using namespace TorConfig;

ManualConfigPage::ManualConfigPage(QWidget *parent)
	: QWizardPage(parent)
{
	setButtonText(QWizard::CustomButton1, tr("Verify Connection"));

	QBoxLayout *layout = new QVBoxLayout(this);

	QLabel *desc = new QLabel;
	desc->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	desc->setWordWrap(true);
	desc->setTextFormat(Qt::RichText);
	desc->setText(tr(
		"TorIM requires a Tor controller connection instead of a normal proxy connection. "
		"This is configured with the <i>ControlPort</i> and <i>HashedControlPassword</i> options in the "
		"Tor configuration. You must set these options in your Tor configuration, and input them here."
	));

	layout->addWidget(desc);
	layout->addSpacing(20);

	QFormLayout *formLayout = new QFormLayout;
	layout->addLayout(formLayout);

	/* IP */
	ipEdit = new QLineEdit;
	ipEdit->setWhatsThis(tr("The IP of the Tor control connection"));

	QRegExpValidator *validator = new QRegExpValidator(QRegExp(QString(
			"^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$")),
			ipEdit);
	ipEdit->setValidator(validator);

	registerField(QString("controlIp*"), ipEdit);
	formLayout->addRow(tr("Control IP"), ipEdit);

	/* Port */
	portEdit = new QLineEdit;
	portEdit->setValidator(new QIntValidator(1, 65535, portEdit));
	portEdit->setWhatsThis(tr("The port used for the Tor control connection (ControlPort option)"));

	registerField(QString("controlPort*"), portEdit);
	formLayout->addRow(tr("Control Port"), portEdit);

	/* Password */
	QLineEdit *passwordEdit = new QLineEdit;
	passwordEdit->setWhatsThis(tr("The password for control authentication. Plaintext of the "
								  "HashedControlPassword option in Tor."));

	registerField(QString("controlPassword"), passwordEdit);
	formLayout->addRow(tr("Control Password"), passwordEdit);

	/* Tester */
	QBoxLayout *testLayout = new QHBoxLayout;

	torTest = new TorConnTestWidget;
	testLayout->addWidget(torTest, 1, Qt::AlignVCenter | Qt::AlignLeft);

	QPushButton *testBtn = new QPushButton(tr("Test Connection"));
	testLayout->addWidget(testBtn, 0, Qt::AlignVCenter | Qt::AlignRight);

	connect(testBtn, SIGNAL(clicked()), this, SLOT(testSettings()));

	layout->addLayout(testLayout);
}

void ManualConfigPage::initializePage()
{
	wizard()->setOption(QWizard::HaveCustomButton1);

	ipEdit->setText(QString("127.0.0.1"));
	portEdit->setText(QString("9051"));
}

bool ManualConfigPage::isComplete() const
{
	return false;
}

void ManualConfigPage::testSettings()
{
	torTest->startTest(field(QString("controlIp")).toString(), field("controlPort").toString().toInt(),
					   field("controlPassword").toString().toLocal8Bit());
}
