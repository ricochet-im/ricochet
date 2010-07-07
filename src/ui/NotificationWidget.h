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
