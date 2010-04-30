#include "TorConnTestWidget.h"
#include "tor/TorControlManager.h"
#include <QBoxLayout>
#include <QLabel>

using namespace TorConfig;

TorConnTestWidget::TorConnTestWidget(QWidget *parent)
	: QWidget(parent), testManager(0)
{
	QBoxLayout *layout = new QHBoxLayout(this);

	infoLabel = new QLabel;
	infoLabel->setText(tr("The connection has not been tested"));

	layout->addWidget(infoLabel);
}

void TorConnTestWidget::startTest(const QString &host, quint16 port, const QByteArray &authPassword)
{
	if (testManager)
	{
		testManager->disconnect(this);
		testManager->deleteLater();
	}

	testManager = new Tor::TorControlManager(this);
	connect(testManager, SIGNAL(socksReady()), this, SLOT(testSuccess()));

	testManager->setAuthPassword(authPassword);
	testManager->connect(QHostAddress(host), port);

	infoLabel->setText(tr("Testing connection..."));
}

void TorConnTestWidget::testSuccess()
{
	infoLabel->setText(tr("Successfully connected with Tor %1").arg(testManager->torVersion()));
	testManager->deleteLater();
	testManager = 0;
}
