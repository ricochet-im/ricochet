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

FancyTextEdit::FancyTextEdit(QWidget *parent)
    : QTextEdit(parent), m_placeholderActive(false)
{
    m_storedColor = textColor();
}

bool FancyTextEdit::isTextEmpty() const
{
    return isPlaceholderActive() || document()->toPlainText().isEmpty();
}

void FancyTextEdit::focusInEvent(QFocusEvent *e)
{
    QTextEdit::focusInEvent(e);

    if (m_placeholderActive)
    {
        clear();
        setTextColor(m_storedColor);

        m_placeholderActive = false;
    }
}

void FancyTextEdit::focusOutEvent(QFocusEvent *e)
{
    QTextEdit::focusOutEvent(e);

    setPlaceholderText(placeholderText());
}

void FancyTextEdit::setPlaceholderText(const QString &placeholderText)
{
    m_placeholderText = placeholderText;

    if (isPlaceholderActive() || (!hasFocus() && this->document()->toPlainText().isEmpty()))
    {
        setTextColor(Qt::gray);
        setText(placeholderText);

        m_placeholderActive = true;
    }
}

const QString &FancyTextEdit::placeholderText() const
{
    return m_placeholderText;
}
