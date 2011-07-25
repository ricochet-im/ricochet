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

#ifndef UIHELPER_H
#define UIHELPER_H

#include <QObject>
#include <QGraphicsProxyWidget>
#include <QDeclarativeItem>

class ChatTextWidget;
class ContactUser;

class UIHelper : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString clipboardText READ clipboardText WRITE setClipboardText NOTIFY clipboardTextChanged)

public:
    explicit UIHelper(QObject *parent = 0);

    static QPoint scenePosToGlobal(const QGraphicsScene *scene, const QPointF &scenePos);

    QString clipboardText() const;
    void setClipboardText(const QString &text);

    Q_INVOKABLE ChatTextWidget *createChatArea(ContactUser *user, QDeclarativeItem *proxyItem);

signals:
    void clipboardTextChanged();
};

class DeclarativeProxiedProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT

public:
    DeclarativeProxiedProxyWidget(QDeclarativeItem *proxyItem, QWidget *widget)
        : QGraphicsProxyWidget(proxyItem), proxyItem(proxyItem)
    {
        widget->setProperty("declarativeProxyWidget", QVariant::fromValue((QObject*)this));
        setWidget(widget);
        updateWidgetGeometry();

        connect(proxyItem, SIGNAL(widthChanged()), SLOT(updateWidgetGeometry()));
        connect(proxyItem, SIGNAL(heightChanged()), SLOT(updateWidgetGeometry()));
    }

    QPoint mapToScreen(const QPoint &pos)
    {
        return UIHelper::scenePosToGlobal(scene(), mapToScene(pos));
    }

public slots:
    void updateWidgetGeometry()
    {
        setGeometry(QRectF(0, 0, proxyItem->width(), proxyItem->height()));
    }

private:
    QDeclarativeItem *proxyItem;
};

#endif // UIHELPER_H
