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

#include "HiddenService.h"
#include "TorControl.h"
#include "TorSocket.h"
#include "utils/CryptoKey.h"
#include "utils/Useful.h"

using namespace Tor;

HiddenService::HiddenService(QObject *parent)
    : QObject(parent), m_status(NotCreated)
{
}

HiddenService::HiddenService(const CryptoKey &privateKey, QObject *parent)
    : QObject(parent), m_status(NotCreated)
{
    setPrivateKey(privateKey);
    m_status = Offline;
}

void HiddenService::setStatus(Status newStatus)
{
    if (m_status == newStatus)
        return;

    Status old = m_status;
    m_status = newStatus;

    emit statusChanged(m_status, old);

    if (m_status == Online)
        emit serviceOnline();
}

void HiddenService::addTarget(const Target &target)
{
    m_targets.append(target);
}

void HiddenService::addTarget(quint16 servicePort, QHostAddress targetAddress, quint16 targetPort)
{
    Target t = { targetAddress, servicePort, targetPort };
    m_targets.append(t);
}

void HiddenService::setPrivateKey(const CryptoKey &key)
{
    if (m_privateKey.isLoaded()) {
        TEGO_BUG() << "Cannot change the private key on an existing HiddenService";
        return;
    }

    if (!key.isPrivate()) {
        TEGO_BUG() << "Cannot create a hidden service with a public key";
        return;
    }

    m_privateKey = key;
    m_hostname = QString::fromUtf8(m_privateKey.torServiceID()) + QStringLiteral(".onion");
    emit privateKeyChanged();
}

void HiddenService::servicePublished()
{
    if (m_hostname.isEmpty()) {
        qDebug() << "Failed to read hidden service hostname";
        return;
    }

    qDebug() << "Hidden service published successfully";
    setStatus(Online);
}

