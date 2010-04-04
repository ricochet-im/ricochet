#include "main.h"
#include "HomeContactWidget.h"
#include <QMouseEvent>
#include <QPainter>

HomeContactWidget::HomeContactWidget(QWidget *parent)
	: QWidget(parent), pSelected(false)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
	setToolTip(tr("Your Information", "Tooltip for the 'Home' button in the contact list"));
}

void HomeContactWidget::setSelected(bool value)
{
	if (pSelected == value)
		return;

	pSelected = value;

	emit selectionChanged(pSelected);
	if (pSelected)
		emit selected();
	else
		emit deselected();

	update();
}

QSize HomeContactWidget::sizeHint() const
{
	return QSize(24, 24);
}

void HomeContactWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() != Qt::LeftButton)
	{
		QWidget::mousePressEvent(event);
		return;
	}

	setSelected();
}

void HomeContactWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter p(this);
	QRect r = rect();

	if (isSelected())
		p.fillRect(r, Qt::blue);

	QPixmap icon(":/icons/home.png");
	p.drawPixmap((r.width() - icon.width()) / 2, (r.height() - icon.height() - 4), icon);
}
