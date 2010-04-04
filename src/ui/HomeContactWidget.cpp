#include "main.h"
#include "HomeContactWidget.h"
#include <QPainter>

HomeContactWidget::HomeContactWidget(QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
}

QSize HomeContactWidget::sizeHint() const
{
	return QSize(24, 24);
}

void HomeContactWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter p(this);
	QRect r = rect();

	QPixmap icon(":/icons/home.png");
	p.drawPixmap((r.width() - icon.width()) / 2, (r.height() - icon.height() - 4), icon);
}
