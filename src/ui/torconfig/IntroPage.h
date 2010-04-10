#ifndef INTROPAGE_H
#define INTROPAGE_H

#include <QWizardPage>

namespace TorConfig
{

class IntroPage : public QWizardPage
{
	Q_OBJECT
	Q_DISABLE_COPY(IntroPage)

public:
	explicit IntroPage(QWidget *parent = 0);

	virtual void initializePage();

	virtual bool isComplete() const;
	virtual int nextId() const;

private slots:
	void setConfigChoice(int choice);

private:
	int configChoice;
};

}

#endif // INTROPAGE_H
