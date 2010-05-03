/* TorIM - http://gitorious.org/torim
 * Copyright (C) 2010, Robin Burchell <robin.burchell@collabora.co.uk>
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

#include "ui/FancyTextEdit.h"

FancyTextEdit::FancyTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
    m_storedColor = textColor();
}

void FancyTextEdit::focusInEvent(QFocusEvent *e)
{
    QTextEdit::focusInEvent(e);

    if (document()->toPlainText() == placeholderText())
    {
        setText("");
        setTextColor(m_storedColor);
    }
}

void FancyTextEdit::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);

    if (this->document()->toPlainText().length() == 0)
        setPlaceholderText(placeholderText());
}

void FancyTextEdit::setPlaceholderText(const QString &placeholderText)
{
    m_placeholderText = placeholderText;

    if (hasFocus())
        return; // don't touch the text of a focused widget. bad karma.

    // set it grey if there is no real text
    setTextColor(Qt::gray);
    setText(placeholderText);
}

const QString &FancyTextEdit::placeholderText() const
{
    return m_placeholderText;
}
