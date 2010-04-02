#ifndef HOMECONTACTWIDGET_H
#define HOMECONTACTWIDGET_H

#include <QWidget>

class HomeContactWidget : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(HomeContactWidget)

public:
	explicit HomeContactWidget(QWidget *parent = 0);

	virtual QSize sizeHint() const;

protected:
	virtual void paintEvent(QPaintEvent *event);
};

#endif // HOMECONTACTWIDGET_H
