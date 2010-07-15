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

#include "main.h"
#include "ChatTextWidget.h"
#include <QScrollBar>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QFontDialog>

static const int defaultBacklog = 125;

ChatTextWidget::ChatTextWidget(QWidget *parent)
    : QTextEdit(parent)
{
    setReadOnly(true);
    setFont(config->value("ui/chatFont", QFont(QLatin1String("Calibri"), 10)).value<QFont>());
    document()->setMaximumBlockCount(config->value("ui/chatBacklog", defaultBacklog).toInt());

    connect(verticalScrollBar(), SIGNAL(rangeChanged(int,int)), SLOT(scrollToBottom()));
}

void ChatTextWidget::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatTextWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(tr("Change Font"), this, SLOT(showFontDialog()));

    menu->exec(e->globalPos());
    delete menu;
}

void ChatTextWidget::setFont(const QFont &font)
{
    QTextEdit::setFont(font);
    emit fontChanged(font);
}

void ChatTextWidget::showFontDialog()
{
    QFont newFont = QFontDialog::getFont(0, font(), this);
    config->setValue("ui/chatFont", newFont);
    setFont(newFont);
}
