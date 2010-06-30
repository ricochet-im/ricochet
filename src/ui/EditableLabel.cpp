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
    originalPalette = palette();
    setFocusPolicy(Qt::NoFocus);

    stopEditing();

    connect(this, SIGNAL(editingFinished()), SLOT(stopEditing()));
}

void EditableLabel::startEditing()
{
    setReadOnly(false);
    setFrame(true);
    setTextMargins(originalMargins);
    QLineEdit::setPalette(originalPalette);
}

void EditableLabel::stopEditing()
{
    QPalette p = originalPalette;
    p.setBrush(QPalette::Base, p.window());
    p.setBrush(QPalette::Text, p.windowText());
    QLineEdit::setPalette(p);

    setReadOnly(true);
    setFrame(false);
    setTextMargins(-2, 0, 0, 0);
    deselect();
}

void EditableLabel::setPalette(const QPalette &nPalette)
{
    originalPalette = nPalette;
    QPalette p = nPalette;

    if (!isEditing())
    {
        p.setBrush(QPalette::Base, p.window());
        p.setBrush(QPalette::Text, p.windowText());
    }

    QLineEdit::setPalette(p);
}

void EditableLabel::mouseDoubleClickEvent(QMouseEvent *ev)
{
    ev->accept();
    startEditing();
    setFocus();
    selectAll();
}
