#ifndef MANUALCONFIGPAGE_H
#define MANUALCONFIGPAGE_H

#include <QWizardPage>

class QLineEdit;

namespace TorConfig
{

class ManualConfigPage : public QWizardPage
{
	Q_OBJECT
	Q_DISABLE_COPY(ManualConfigPage)

public:
	explicit ManualConfigPage(QWidget *parent = 0);

	virtual void initializePage();

	virtual bool isComplete() const;

private:
	QLineEdit *ipEdit, *portEdit;
};

}

#endif // MANUALCONFIGPAGE_H
