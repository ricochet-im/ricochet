#include "TorConnTestWidget.h"
#include "tor/TorControlManager.h"
#include <QBoxLayout>
#include <QLabel>

using namespace TorConfig;

TorConnTestWidget::TorConnTestWidget(QWidget *parent)
	: QWidget(parent), testManager(0), m_state(-1)
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

	m_state = -1;

	testManager = new Tor::TorControlManager(this);
	connect(testManager, SIGNAL(socksReady()), this, SLOT(doTestSuccess()));
	connect(testManager, SIGNAL(statusChanged(int,int)), this, SLOT(torStatusChanged(int)));

	testManager->setAuthPassword(authPassword);
	testManager->connect(QHostAddress(host), port);

	infoLabel->setText(tr("Testing connection..."));

	emit testStarted();
	emit stateChanged();
}

void TorConnTestWidget::clear()
{
	if (testManager)
	{
		testManager->disconnect(this);
		testManager->deleteLater();
		testManager = 0;
	}

	m_state = -1;
	infoLabel->setText(tr("The connection has not been tested"));
	emit stateChanged();
}

void TorConnTestWidget::doTestSuccess()
{
	infoLabel->setText(tr("Successfully connected with Tor %1").arg(testManager->torVersion()));
	testManager->deleteLater();
	testManager = 0;

	m_state = 1;
	emit testSucceeded();
	emit testFinished(true);
	emit stateChanged();
}

void TorConnTestWidget::doTestFail()
{
	infoLabel->setText(testManager->statusText());
	testManager->deleteLater();
	testManager = 0;

	m_state = 0;
	emit testFailed();
	emit testFinished(false);
	emit stateChanged();
}

void TorConnTestWidget::torStatusChanged(int status)
{
	if (status == Tor::TorControlManager::Error)
		doTestFail();
}
