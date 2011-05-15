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

#include "PopoutManager.h"
#include <QGraphicsView>
#include <QApplication>
#include <QResizeEvent>

PopoutManager::PopoutManager(QGraphicsView *w)
    : QObject(w), mainWindow(w), wPos(0)
{
}

QObject *PopoutManager::createWindow(const QRectF &rect)
{
    QSize desiredSize = rect.size().toSize();
    QPoint desiredPos = scenePosToGlobal(rect.topLeft());

    wPos -= 100000;

    PopoutWindow *w = new PopoutWindow();
    w->setScene(mainWindow->scene());
    w->setSceneRect(QRectF(wPos, wPos, rect.width(), rect.height()));
    w->resize(desiredSize);
    if (!desiredPos.isNull())
        w->move(desiredPos);
    w->show();

    /* Attempt to move the window again to position its contents at exactly
     * the screen coordinates they used before, accounting for window frame
     * geometry */
    QPoint frame(w->geometry().x() - w->x(), w->geometry().y() - w->y());
    if (!desiredPos.isNull() && (frame.x() > 0 || frame.y() > 0))
        w->move(desiredPos - frame);

    return w;
}

QPoint PopoutManager::scenePosToGlobal(const QPointF &scenePos)
{
    /* Try the active window first */
    QGraphicsView *v = qobject_cast<QGraphicsView*>(QApplication::activeWindow());
    if (v && v->scene() == mainWindow->scene() && v->sceneRect().contains(scenePos))
        return v->viewport()->mapToGlobal(v->mapFromScene(scenePos));

    /* Try all other views of this scene */
    foreach (v, mainWindow->scene()->views())
    {
        /* Assuming that only one view is visualizing any part of the scene
         * at a time; if this doesn't hold true and the activeWindow test fails,
         * this may position against the incorrect window. */
        if (v->sceneRect().contains(scenePos))
            return v->viewport()->mapToGlobal(v->mapFromScene(scenePos));
    }

    return QPoint();
}

PopoutWindow::PopoutWindow(QWidget *parent)
    : QGraphicsView(parent)
{
    setWindowFlags(windowFlags() | Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    setFrameStyle(QFrame::NoFrame);

    setOptimizationFlags(QGraphicsView::DontSavePainterState);
    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    viewport()->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::StrongFocus);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void PopoutWindow::resizeEvent(QResizeEvent *event)
{
    if (event->oldSize().width() != event->size().width())
        emit widthChanged(event->size().width());
    if (event->oldSize().height() != event->size().height())
        emit heightChanged(event->size().height());
    QGraphicsView::resizeEvent(event);
}

void PopoutWindow::closeEvent(QCloseEvent *event)
{
    QGraphicsView::closeEvent(event);
    if (event->isAccepted())
        emit closed();
}
