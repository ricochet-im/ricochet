/* Torsion - http://torsionim.org/
 * Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "ExpandingTextEdit.h"

ExpandingTextEdit::ExpandingTextEdit(QWidget *parent)
    : QTextEdit(parent), m_maxLength(-1), m_maxLines(-1), m_currentLength(0), m_currentLines(0)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(document(), SIGNAL(contentsChanged()), SLOT(documentChanged()));
}

QSize ExpandingTextEdit::sizeHint() const
{
    QSize sz = document()->size().toSize();
    QRect cr = contentsRect();
    sz += QSize(cr.x()*2, cr.y()*2);
    return sz;
}

QSize ExpandingTextEdit::minimumSizeHint() const
{
    return QSize();
}

void ExpandingTextEdit::resizeEvent(QResizeEvent *event)
{
    QTextEdit::resizeEvent(event);
    document()->setTextWidth(viewport()->width());
    updateGeometry();
}

void ExpandingTextEdit::setMaxLength(int maxLength)
{
    m_maxLength = maxLength;
}

void ExpandingTextEdit::setMaxLines(int maxLines)
{
    m_maxLines = maxLines;
}

#include <QDebug>

void ExpandingTextEdit::documentChanged()
{
    static bool isActive = false;
    if (isActive)
        return;

    isActive = true;

    m_currentLength = document()->characterCount();

    if (m_maxLength >= 0 && m_currentLength > m_maxLength)
    {
        int del = m_currentLength - m_maxLength;

        QTextCursor cursor = textCursor();
        cursor.clearSelection();

        int p = cursor.position();
        cursor.setPosition(qMax(0, p - del), QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        del = qMax(0, del - p);

        if (del > 0)
        {
            cursor.setPosition(p + del, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
        }

        m_currentLength = document()->characterCount();
        Q_ASSERT(m_currentLength == m_maxLength);
    }

    int lines = document()->lineCount();
    if (m_maxLines >= 0 && lines > m_maxLines)
    {
        QTextCursor cursor = textCursor();
        cursor.clearSelection();

        while (lines > m_maxLines)
        {
            if (!cursor.atStart())
            {
                cursor.movePosition(QTextCursor::Up, QTextCursor::KeepAnchor);
                cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
            }
            else if (!cursor.atEnd())
            {
                cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor);
                cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
            }

            Q_ASSERT(cursor.hasSelection());
            cursor.removeSelectedText();

            --lines;
            Q_ASSERT(lines == document()->lineCount());
        }
    }

    if (lines != m_currentLines)
    {
        m_currentLines = lines;
        updateGeometry();
    }

    isActive = false;
}
