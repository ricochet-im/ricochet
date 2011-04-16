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

#ifndef EXPANDINGTEXTEDIT_H
#define EXPANDINGTEXTEDIT_H

#include <QTextEdit>

class ExpandingTextEdit : public QTextEdit
{
    Q_OBJECT

    Q_PROPERTY(int maxLength READ maxLength WRITE setMaxLength)
    Q_PROPERTY(int maxLines READ maxLines WRITE setMaxLines)

public:
    explicit ExpandingTextEdit(QWidget *parent = 0);

    virtual QSize sizeHint() const;
    virtual QSize minimumSizeHint() const;

    int characterCount() const { return m_currentLength; }
    int lineCount() const { return m_currentLines; }

    int maxLength() const { return m_maxLength; }
    int maxLines() const { return m_maxLines; }

    bool canInput() const;

public slots:
    void setMaxLength(int maxLength);
    void setMaxLines(int maxLines);

private slots:
    void documentChanged();

protected:
    virtual void resizeEvent(QResizeEvent *e);

private:
    int m_maxLength, m_maxLines;
    int m_currentLength, m_currentLines;
};

#endif // EXPANDINGTEXTEDIT_H
