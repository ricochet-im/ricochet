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

#include "main.h"
#include "ChatTextWidget.h"
#include <QScrollBar>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>
#include <QFontDialog>
#include <QApplication>
#include <QWindowsVistaStyle>
#include <QShortcut>

static const int defaultBacklog = 125;

ChatTextWidget::ChatTextWidget(QWidget *parent)
    : QTextEdit(parent)
{
    setReadOnly(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFocusPolicy(static_cast<Qt::FocusPolicy>(focusPolicy() & ~Qt::TabFocus));
    document()->setMaximumBlockCount(config->value("ui/chatBacklog", defaultBacklog).toInt());

#ifdef Q_OS_WIN
    if (qobject_cast<QWindowsVistaStyle*>(style()))
    {
        QFont defaultFont(QLatin1String("Segoe UI, MS Shell Dlg 2"), 9);
        setFont(config->value("ui/chatFont", defaultFont).value<QFont>());
    }
    else
#endif
        setFont(config->value("ui/chatFont", QApplication::font(this)).value<QFont>());

    connect(verticalScrollBar(), SIGNAL(rangeChanged(int,int)), SLOT(scrollToBottom()));

    config->addTrackingProperty(QLatin1String("ui/chatFont"), this, "font");

    new QShortcut(QKeySequence(Qt::Key_F10), this, SLOT(clear()));
}

void ChatTextWidget::scrollToBottom()
{
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void ChatTextWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = createStandardContextMenu(e->pos());
    menu->addAction(tr("Clear"), this, SLOT(clear()), QKeySequence(Qt::Key_F10));
    menu->addSeparator();
    menu->addAction(tr("Change Font"), this, SLOT(showFontDialog()));

    menu->exec(e->globalPos());
    delete menu;
}

bool ChatTextWidget::event(QEvent *e)
{
    bool re = QTextEdit::event(e);

    if (e->type() == QEvent::FontChange)
        emit fontChanged(font());

    return re;
}

void ChatTextWidget::showFontDialog()
{
    QFont newFont = QFontDialog::getFont(0, font(), this);
    config->setValue("ui/chatFont", newFont);
    setFont(newFont);
}
