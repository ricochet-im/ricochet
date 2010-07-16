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

#ifndef CHATTEXTWIDGET_H
#define CHATTEXTWIDGET_H

#include <QTextEdit>

class ChatTextWidget : public QTextEdit
{
    Q_OBJECT
    Q_DISABLE_COPY(ChatTextWidget)

    Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY fontChanged STORED true)

public:
    explicit ChatTextWidget(QWidget *parent = 0);

public slots:
    void scrollToBottom();
    void showFontDialog();

signals:
    void fontChanged(const QFont &font);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual bool event(QEvent *e);
};

#endif // CHATTEXTWIDGET_H
