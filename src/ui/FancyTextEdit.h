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

#ifndef FANCYTEXTEDIT_H
#define FANCYTEXTEDIT_H

#include <QTextEdit>

class FancyTextEdit : public QTextEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(FancyTextEdit)

public:
    explicit FancyTextEdit(QWidget *parent = 0);

    void setPlaceholderText(const QString &placeholderText);
    const QString &placeholderText() const;

    bool isPlaceholderActive() const { return m_placeholderActive; }
    bool isTextEmpty() const;

protected:
    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);

private:
    QColor m_storedColor;
    QString m_placeholderText;
    bool m_placeholderActive;
};

#endif // FANCYTEXTEDIT_H
