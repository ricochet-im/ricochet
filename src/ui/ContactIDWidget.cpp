/* Torsion - http://github.com/special/torsion
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
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
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#include "ContactIDWidget.h"
#include <QMouseEvent>

ContactIDWidget::ContactIDWidget(QWidget *parent)
    : QLineEdit(parent), blockMousePress(false)
{
    setReadOnly(true);
    setFont(idFont());
}

QFont ContactIDWidget::idFont()
{
    QFont font = QFont(QLatin1String("Consolas, \"Courier New\""), 9);
    font.setStyleHint(QFont::TypeWriter);
    return font;
}

void ContactIDWidget::focusInEvent(QFocusEvent *ev)
{
    QLineEdit::focusInEvent(ev);
    selectAll();

    if (ev->reason() == Qt::MouseFocusReason)
        blockMousePress = true;
}

void ContactIDWidget::mousePressEvent(QMouseEvent *ev)
{
    if (blockMousePress)
    {
        blockMousePress = false;
        ev->accept();
        return;
    }

    QLineEdit::mousePressEvent(ev);
}

void ContactIDWidget::mouseDoubleClickEvent(QMouseEvent *ev)
{
    selectAll();
    copy();

    ev->accept();
}
