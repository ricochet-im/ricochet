#ifndef TORCONFIGWIZARD_H
#define TORCONFIGWIZARD_H

#include <QWizard>

class TorConfigWizard : public QWizard
{
	Q_OBJECT
	Q_DISABLE_COPY(TorConfigWizard)

public:
	explicit TorConfigWizard(QWidget *parent = 0);

	virtual void accept();
};

#endif // TORCONFIGWIZARD_H
