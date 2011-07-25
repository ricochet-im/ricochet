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

#include "UIHelper.h"
#include "ui/ChatTextWidget.h"
#include <QTextEdit>
#include <QDeclarativeItem>
#include <QGraphicsView>
#include <QApplication>
#include <QClipboard>

UIHelper::UIHelper(QObject *parent)
    : QObject(parent)
{
    connect(qApp->clipboard(), SIGNAL(changed(QClipboard::Mode)), SIGNAL(clipboardTextChanged()));
}

ChatTextWidget *UIHelper::createChatArea(ContactUser *user, QDeclarativeItem *proxyItem)
{
    ChatTextWidget *text = new ChatTextWidget(user);
    text->setFrameStyle(QFrame::NoFrame);

    DeclarativeProxiedProxyWidget *w = new DeclarativeProxiedProxyWidget(proxyItem, text);
    Q_UNUSED(w);

    return text;
}

QString UIHelper::clipboardText() const
{
    return qApp->clipboard()->text();
}

void UIHelper::setClipboardText(const QString &text)
{
    qApp->clipboard()->setText(text);
}

QPoint UIHelper::scenePosToGlobal(const QGraphicsScene *scene, const QPointF &scenePos)
{
    /* Try the active window first */
    QGraphicsView *v = qobject_cast<QGraphicsView*>(QApplication::activeWindow());
    if (v && v->scene() == scene && v->sceneRect().contains(scenePos))
        return v->viewport()->mapToGlobal(v->mapFromScene(scenePos));

    /* Try all other views of this scene */
    foreach (v, scene->views())
    {
        /* Assuming that only one view is visualizing any part of the scene
         * at a time; if this doesn't hold true and the activeWindow test fails,
         * this may position against the incorrect window. */
        if (v->sceneRect().contains(scenePos))
            return v->viewport()->mapToGlobal(v->mapFromScene(scenePos));
    }

    return QPoint();
}
