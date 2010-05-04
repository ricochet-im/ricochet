#include "ContactRequestClient.h"
#include "core/ContactUser.h"
#include "ProtocolManager.h"
#include "IncomingSocket.h"
#include "CommandDataParser.h"
#include "tor/TorControlManager.h"
#include "tor/HiddenService.h"
#include "utils/CryptoKey.h"
#include <QNetworkProxy>
#include <QtEndian>

ContactRequestClient::ContactRequestClient(ContactUser *u)
    : QObject(u), user(u), state(NotConnected)
{
    connect(&socket, SIGNAL(connected()), this, SLOT(socketConnected()));
    connect(&socket, SIGNAL(readyRead()), this, SLOT(socketReadable()));
}

void ContactRequestClient::setMessage(const QString &message)
{
    m_message = message;
}

void ContactRequestClient::setMyNickname(const QString &nick)
{
    m_mynick = nick;
}

void ContactRequestClient::sendRequest()
{
    if (!torManager->isSocksReady())
    {
        /* Impossible to send now, requests are triggered when socks becomes ready */
        return;
    }

    socket.setProxy(torManager->connectionProxy());
    socket.connectToHost(user->conn()->host(), user->conn()->port());
}

void ContactRequestClient::socketConnected()
{
    socket.write(IncomingSocket::introData(0x80));
    state = WaitCookie;

    qDebug() << "Contact request for" << user->uniqueID << "connected";
}

void ContactRequestClient::socketReadable()
{
    switch (state)
    {
    case WaitCookie:
        if (socket.bytesAvailable() < 16)
            return;

        buildRequestData(socket.read(16));
        break;

    case WaitAck:
    case WaitResponse:
        break;
    }
}

void ContactRequestClient::buildRequestData(QByteArray cookie)
{
    /* [2*length][16*hostname][data:pubkey][data:signedcookie][str:nick][str:message] */
    QByteArray requestData;
    CommandDataParser request(&requestData);

    /* Hostname */
    Tor::HiddenService *service = torManager->hiddenServices().value(0);

    QString hostname = service ? service->hostname() : QString();
    hostname.truncate(hostname.lastIndexOf(QChar('.')));
    if (hostname.size() != 16)
    {
        qWarning() << "Cannot send contact request: unable to determine the local service hostname";
        /* TODO handle this somehow */
        return;
    }

    /* Public service key */
    CryptoKey serviceKey = service->cryptoKey();
    if (!serviceKey.isValid())
    {
        qWarning() << "Cannot send contact request: failed to load service key";
        /* TODO also */
        return;
    }

    QByteArray publicKeyData = serviceKey.encodedPublicKey();
    if (publicKeyData.isNull())
    {
        qWarning() << "Cannot send contact request: failed to encode service key";
        /* TODO .. */
        return;
    }

    /* Signed cookie */
    QByteArray signature = serviceKey.signData(cookie);
    if (signature.isNull())
    {
        qWarning() << "Cannot send contact request: failed to sign cookie";
        /* TODO again! */
        return;
    }

    request.writeVariableData(signature);

    /* Build request */
    request << (quint16)0; /* placeholder for length */
    request.writeFixedData(hostname.toLatin1());
    request.writeVariableData(publicKeyData);
    request.writeVariableData(signature);
    request << myNickname() << message();

    if (request.hasError())
    {
        qWarning() << "Cannot send contact request: command building failed";
        /* TODO something */
        return;
    }

    /* Set length */
    qToBigEndian((quint16)requestData.size(), reinterpret_cast<uchar*>(requestData.data()));

    /* Send */
    qint64 re = socket.write(requestData);
    Q_ASSERT(re == requestData.size());

    qDebug() << "Contact request for" << user->uniqueID << "sent request data";
}
