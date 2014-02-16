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

#ifndef PROTOCOLMANAGER_H
#define PROTOCOLMANAGER_H

#include <QObject>
#include <QList>
#include <QQueue>
#include <QHash>
#include <QAbstractSocket>
#include <QTimer>
#include "ProtocolSocket.h"

class ContactUser;

class ProtocolManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ProtocolManager)

    Q_PROPERTY(QString host READ host WRITE setHost STORED true)
    Q_PROPERTY(quint16 port READ port WRITE setPort STORED true)

public:
    ContactUser * const user;

    explicit ProtocolManager(ContactUser *user, const QString &host, quint16 port);

    QString host() const { return pHost; }
    void setHost(const QString &host);
    quint16 port() const { return pPort; }
    void setPort(quint16 port);
    QByteArray secret() const { return pSecret; }
    void setSecret(const QByteArray &secret);

    bool isPrimaryConnected() const;
    bool isAnyConnected() const;
    bool isConnectable() const { return !pHost.isEmpty() && !pSecret.isEmpty() && pPort; }

    ProtocolSocket *primary() { return pPrimary; }

    void addSocket(QTcpSocket *socket, ProtocolSocket::Purpose purpose);

public slots:
    void connectPrimary();
    void disconnectAll();

signals:
    void primaryConnected();
    void primaryDisconnected();

private slots:
    void onPrimaryConnected();
    void onPrimaryDisconnected();

    void spawnReconnect();

private:
    ProtocolSocket *pPrimary, *remotePrimary;

    QString pHost;
    QByteArray pSecret;
    quint16 pPort;

    QTimer connectTimer;
    int connectAttempts;

    void setPrimary(ProtocolSocket *newPrimary);
};

/* Do not change this, as it breaks backwards compatibility. Hopefully, it will never be necessary. */
static const quint8 protocolVersion = 0x00;

#endif // PROTOCOLMANAGER_H
