/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * TorIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TorIM. If not, see http://www.gnu.org/licenses/
 */

#ifndef HOMECONTACTWIDGET_H
#define HOMECONTACTWIDGET_H

#include <QWidget>

class HomeContactWidget : public QWidget
{
	Q_OBJECT
	Q_DISABLE_COPY(HomeContactWidget)

	friend class QPropertyAnimation;

	Q_PROPERTY(int iconOffset READ iconOffset WRITE setIconOffset)

public:
	explicit HomeContactWidget(QWidget *parent = 0);

	bool isSelected() const { return pSelected; }

	virtual QSize sizeHint() const;

public slots:
	void setSelected(bool selected = true);
	void clearSelected() { setSelected(false); }

signals:
	void selectionChanged(bool selected);
	void selected();
	void deselected();

protected:
	virtual void paintEvent(QPaintEvent *event);
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void enterEvent(QEvent *event);
	virtual void leaveEvent(QEvent *event);

private:
	bool pSelected;
	int pIconOffset;

	int iconOffset() const { return pIconOffset; }
	void setIconOffset(int offset);
};

#endif // HOMECONTACTWIDGET_H
