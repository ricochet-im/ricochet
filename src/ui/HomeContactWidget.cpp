#include "main.h"
#include "HomeContactWidget.h"
#include <QMouseEvent>
#include <QPainter>
#include <QCursor>

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

void HomeContactWidget::enterEvent(QEvent *event)
{
	Q_UNUSED(event);
	update();
}

void HomeContactWidget::leaveEvent(QEvent *event)
{
	Q_UNUSED(event);
	update();
}

void HomeContactWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter p(this);
	QRect r = rect();

	QPixmap icon(":/icons/home.png");

	int xpos = (r.width() - icon.width());

	if (isSelected())
	{
		//p.fillRect(r, Qt::blue);
		xpos /= 2;
	}
	else if (!QRect(mapToGlobal(QPoint(0,0)), size()).contains(QCursor::pos()))
	{
		p.setOpacity(0.5);
		xpos -= 5;
	}
	else
		xpos -= 5;

	p.drawPixmap(xpos, (r.height() - icon.height() - 4), icon);
}
