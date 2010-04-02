#include "main.h"
#include "HomeContactWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>

HomeContactWidget::HomeContactWidget(QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
}

QSize HomeContactWidget::sizeHint() const
{
	return QSize(160, 48);
}

void HomeContactWidget::paintEvent(QPaintEvent *event)
{
	QPainter p(this);

	QRect r = rect();

	/* Selection TBD. */

	r.adjust(5, 5, -5, -5);

	/* Avatar */
	QPixmap avatar;
	avatar.loadFromData(config->value("contacts/dummy3/avatar-tiny").toByteArray());
	QPoint avatarPos = QPoint(r.x() + (35 - avatar.width()) / 2, r.y() + (35 - avatar.height()) / 2);

	if (!avatar.isNull())
	{
		if (avatar.width() > 35 || avatar.height() > 35)
			avatar = avatar.scaled(QSize(35, 35), Qt::KeepAspectRatio, Qt::SmoothTransformation);

		p.drawPixmap(avatarPos, avatar);
	}

	/* Status */
	QPixmap status(":/icons/status-online.png");
	if (!status.isNull())
		p.drawPixmap(avatarPos - QPoint(status.width()/2-1, status.height()/2-1), status);

	/* Draw nickname */
	r.adjust(41, 0, 0, 0);

	QFont nickFont = QFont("Calibri", 11);
	p.setFont(nickFont);

	QString nickname = tr("Home");

	/* Caution: horrifically slow */
	QFontMetrics metrics = p.fontMetrics();
	QRect nickRect = metrics.tightBoundingRect(nickname);

	//p.drawText(r.topLeft() + QPoint(0, nickRect.height()+1), nickname);
	p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, nickname);

#if 0
	/* Draw info text */
	QString infoText("11 months ago");

	QFont infoFont = QFont("Arial", 8);
	p.setFont(infoFont);

	p.setPen(Qt::gray);
	p.drawText(r.bottomLeft() - QPoint(0, 1), infoText);
#endif
}
