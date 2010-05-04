#include "ContactRequestClient.h"
#include "core/ContactUser.h"
#include "ProtocolManager.h"
#include "IncomingSocket.h"
#include "CommandDataParser.h"
#include "tor/TorControlManager.h"
#include "tor/HiddenService.h"
#include <QNetworkProxy>

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

    /* Placeholder for length */
    request << (quint16)0;

    /* Hostname; ASCII */
    QString hostname = torManager->hiddenServices()[0]->hostname();
    hostname.truncate(hostname.lastIndexOf(QChar('.')));
    if (hostname.size() != 16)
    {
        qWarning() << "Cannot send contact request: unable to determine the local service hostname";
        /* TODO handle this somehow */
        return;
    }

    request.writeFixedData(hostname.toLatin1());
}
