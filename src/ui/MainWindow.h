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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDeclarativeView>
#include <QWeakPointer>
#include <QPointer>

class ContactUser;
class UserIdentity;
class IncomingContactRequest;
class OutgoingContactRequest;

class MainWindow : public QDeclarativeView
{
    Q_OBJECT
    Q_DISABLE_COPY(MainWindow)

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void showNotification(const QString &message, QObject *receiver = 0, const char *slot = 0);

public slots:
    void openTorConfig();

    void uiRemoveContact(ContactUser *user);

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    /* Incoming contact request notifications */
    void updateContactRequests();
    void showContactRequest();

    /* Outgoing contact request notifications */
    void outgoingRequestAdded(OutgoingContactRequest *request);
    void updateOutgoingRequest(OutgoingContactRequest *request = 0);

    /* Tor status notifications */
    void updateTorStatus();

private:
    void createContactsModel();
};

extern MainWindow *uiMain;

#endif // MAINWINDOW_H
