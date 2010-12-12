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
