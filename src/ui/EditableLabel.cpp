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

#include "EditableLabel.h"
#include <QMouseEvent>

EditableLabel::EditableLabel(QWidget *parent)
    : QLineEdit(parent)
{
    originalMargins = textMargins();
    setFocusPolicy(Qt::NoFocus);

    stopEditing();

    connect(this, SIGNAL(editingFinished()), SLOT(stopEditing()));
}

void EditableLabel::startEditing()
{
    setReadOnly(false);
    setFrame(true);
    setTextMargins(originalMargins);
    setPalette(originalPalette);
}

void EditableLabel::stopEditing()
{
    setReadOnly(true);
    setFrame(false);
    setTextMargins(-2, 0, 0, 0);

    originalPalette = palette();
    QPalette p = originalPalette;
    p.setBrush(QPalette::Base, p.window());
    p.setBrush(QPalette::Text, p.windowText());
    setPalette(p);
}

void EditableLabel::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ev->accept();
    startEditing();
    setFocus();
}

bool EditableLabel::event(QEvent *ev)
{
    if (ev->type() == QEvent::PaletteChange)
    {
        if (!isEditing())
            originalPalette = palette();
    }

    return QWidget::event(ev);
}
