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

#ifndef POPOUTMANAGER_H
#define POPOUTMANAGER_H

#include <QObject>
#include <QGraphicsView>
#include "UIHelper.h"

class PopoutManager : public QObject
{
    Q_OBJECT

public:
    QGraphicsView * const mainWindow;

    explicit PopoutManager(QGraphicsView *mainWindow);

public slots:
    QObject *createWindow(const QRectF &rect);

private:
    qreal wPos;

    QPoint scenePosToGlobal(const QPointF &scenePos)
    {
        return UIHelper::scenePosToGlobal(mainWindow->scene(), scenePos);
    }
};

/* We can probably avoid PopoutManager entirely by making this constructable.. */
class PopoutWindow : public QGraphicsView
{
    Q_OBJECT

    Q_PROPERTY(int width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(int height READ height WRITE setHeight NOTIFY heightChanged)
    Q_PROPERTY(qreal sceneX READ sceneX)
    Q_PROPERTY(qreal sceneY READ sceneY)

public:
    PopoutWindow(QWidget *parent = 0);

    qreal sceneX() const { return sceneRect().x(); }
    qreal sceneY() const { return sceneRect().y(); }

    Q_INVOKABLE void moveOffset(int dx, int dy)
    {
        move(x() + dx, y() + dy);
    }

public slots:
    void setWidth(int w) { resize(w, height()); }
    void setHeight(int h) { resize(width(), h); }
    bool close() { return QGraphicsView::close(); }

signals:
    void widthChanged(int width);
    void heightChanged(int height);
    void closed();

protected:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void closeEvent(QCloseEvent *event);
};

#endif // POPOUTMANAGER_H
