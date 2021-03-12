/* Ricochet - https://ricochet.im/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

namespace shims
{
    class ContactUser;
}
class IncomingContactRequest;
class OutgoingContactRequest;
class QQmlApplicationEngine;
class QQuickItem;
class QQuickWindow;

class MainWindow : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString accessibleVersion READ accessibleVersion CONSTANT)
    Q_PROPERTY(QString aboutText READ aboutText CONSTANT)
    Q_PROPERTY(QVariantMap screens READ screens CONSTANT)

public:
    explicit MainWindow(QObject *parent = 0);
    ~MainWindow();

    bool showUI();

    QString aboutText() const;
    QString version() const;
    QString accessibleVersion() const;
    QVariantMap screens() const;

    Q_INVOKABLE bool showRemoveContactDialog(shims::ContactUser *user);

    // Find parent window of a QQuickItem; exposed as property after Qt 5.4
    Q_INVOKABLE QQuickWindow *findParentWindow(QQuickItem *item);

private:
    QQmlApplicationEngine *qml;
};

extern MainWindow *uiMain;

#endif // MAINWINDOW_H
