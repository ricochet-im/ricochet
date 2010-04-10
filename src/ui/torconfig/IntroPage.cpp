#include "IntroPage.h"
#include <QBoxLayout>
#include <QLabel>
#include <QCommandLinkButton>
#include <QSignalMapper>

using namespace TorConfig;

IntroPage::IntroPage(QWidget *parent)
	: QWizardPage(parent), configChoice(-1)
{
	QBoxLayout *layout = new QVBoxLayout(this);

	/* Introduction */
	QLabel *intro = new QLabel;
	intro->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	intro->setWordWrap(true);
	intro->setText(tr(
		"First, TorIM must be configured to work with your installation of Tor. "
		"To continue, select your desired setup method below."
	));
	layout->addWidget(intro);

	layout->addSpacing(40);

	QSignalMapper *btnMapper = new QSignalMapper(this);
	connect(btnMapper, SIGNAL(mapped(int)), this, SLOT(setConfigChoice(int)));

	/* Vidalia (temporary) */
	QCommandLinkButton *vidaliaBtn = new QCommandLinkButton;
	vidaliaBtn->setText(tr("Use Vidalia (Recommended)"));
	vidaliaBtn->setDescription(tr("Automatically reconfigure Vidalia and Tor to work with TorIM"));

	connect(vidaliaBtn, SIGNAL(clicked()), btnMapper, SLOT(map()));
	btnMapper->setMapping(vidaliaBtn, 1);

	layout->addWidget(vidaliaBtn);

	/* Manual configuration */
	QCommandLinkButton *manualBtn = new QCommandLinkButton;
	manualBtn->setText(tr("Configure manually"));
	manualBtn->setDescription(tr("Manually setup the Tor control connection. Advanced users only."));
	layout->addWidget(manualBtn);

	connect(manualBtn, SIGNAL(clicked()), btnMapper, SLOT(map()));
	btnMapper->setMapping(manualBtn, 2);

	layout->addStretch();
}

void IntroPage::setConfigChoice(int choice)
{
	configChoice = choice;
	wizard()->next();
}

void IntroPage::initializePage()
{
	wizard()->button(QWizard::NextButton)->setVisible(false);
	wizard()->button(QWizard::FinishButton)->setVisible(false);
}

bool IntroPage::isComplete() const
{
	return false;
}

int IntroPage::nextId() const
{
	return configChoice;
}
