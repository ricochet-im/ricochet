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

#ifndef NOTIFICATIONWIDGET_H
#define NOTIFICATIONWIDGET_H

#include <QWidget>

class QTextLayout;

class NotificationWidget : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(NotificationWidget)

public:
    explicit NotificationWidget(QWidget *parent = 0);
    explicit NotificationWidget(const QString &message, QWidget *parent = 0);

    const QString &message() const { return m_message; }
    void setMessage(const QString &message);

    virtual QSize sizeHint() const;
    virtual int heightForWidth(int width) const;

public slots:
    void showAnimated();
    void closeNotification();

signals:
    void clicked();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);

private:
    QString m_message;
    QBrush backgroundBrush;

    void visualSetup();
    void layoutMessage(QTextLayout &layout, const QRect &rect) const;
};

#endif // NOTIFICATIONWIDGET_H
