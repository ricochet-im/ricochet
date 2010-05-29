#include "ContactRequestServer.h"
#include "CommandDataParser.h"
#include "utils/CryptoKey.h"
#include "utils/SecureRNG.h"
#include "core/ContactsManager.h"
#include "core/IncomingRequestManager.h"
#include <QTcpSocket>
#include <QtEndian>

ContactRequestServer::ContactRequestServer(QTcpSocket *s)
    : socket(s), state(WaitRequest)
{
    socket->setParent(this);

    connect(socket, SIGNAL(readyRead()), this, SLOT(socketReadable()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    qDebug() << "Contact request connection created; sending cookie";

    sendCookie();
}

void ContactRequestServer::close()
{
    socket->close();
}

void ContactRequestServer::socketDisconnected()
{
    qDebug() << "Contact request connection closed";
    deleteLater();
}

void ContactRequestServer::sendCookie()
{
    cookie = SecureRNG::random(16);
    Q_ASSERT(cookie.size() == 16);

    qint64 re = socket->write(cookie);
    Q_ASSERT(re == cookie.size());

    state = WaitRequest;
}

/* Returns true if the connection is still open (dependant on the response) */
bool ContactRequestServer::sendResponse(uchar response)
{
    Q_ASSERT(state != SentResponse);

    qint64 re = socket->write(reinterpret_cast<const char*>(&response), 1);
    Q_ASSERT(re);

    if (response != 0x00)
        state = SentResponse;

    if (response > 0x01)
    {
        /* Everything except acknowledge (0x00) and accept (0x01) is an error or rejection */
        socket->close();
        return false;
    }

    return true;
}

void ContactRequestServer::sendAccept(ContactUser *user)
{
    /* Send the accepted response; immediately after this, both ends will treat the connection
     * as their newly established primary. */
    sendResponse(0x01);

    qDebug() << "Contact request accepted with an active connection; sending accept and morphing to primary";

    socket->disconnect(this);
    user->conn()->addSocket(socket, 0x00);
    Q_ASSERT(socket->parent() != this);

    deleteLater();
}

void ContactRequestServer::sendRejection()
{
    sendResponse(0x40);
}

void ContactRequestServer::socketReadable()
{
    if (state != WaitRequest)
    {
        /* After the request has arrived, all data from the client is ignored. */
        socket->readAll();
        return;
    }

    /* Peek at the request length field */
    quint16 length;
    if (socket->peek(reinterpret_cast<char*>(&length), sizeof(length)) < (int)sizeof(length))
        return;

    length = qFromBigEndian(length);
    if (socket->bytesAvailable() < length)
        return;

    QByteArray request = socket->read(length);
    Q_ASSERT(request.size() == length);

    handleRequest(request);
}

void ContactRequestServer::handleRequest(const QByteArray &data)
{
    /* [2*length][16*hostname][16*connSecret][data:pubkey][data:signedcookie][str:nick][str:message] */
    CommandDataParser request(&data);
    request.setPos(2);

    QByteArray hostname, connSecret, encodedPublicKey, signedCookie;
    QString nickname, message;

    request.readFixedData(&hostname, 16);
    request.readFixedData(&connSecret, 16);
    request.readVariableData(&encodedPublicKey);
    request.readVariableData(&signedCookie);
    request >> nickname >> message;

    if (request.hasError())
    {
        qWarning() << "Incoming contact request has a syntax error; rejecting";
        sendResponse(0x80);
        return;
    }

    /* Load the public key */
    CryptoKey key;
    if (!key.loadFromData(encodedPublicKey))
    {
        qWarning() << "Incoming contact request has an unparsable public key; rejecting";
        sendResponse(0x81);
        return;
    }

    /* Verify that the public key corrosponds to the hidden service hostname */
    if (key.torServiceID().toLatin1() != hostname)
    {
        qWarning() << "Incoming contact request hostname does not match the provided public key; rejecting";
        sendResponse(0x81);
        return;
    }

    /* Verify the cookie signature */
    Q_ASSERT(!cookie.isNull());
    if (!key.verifySignature(cookie, signedCookie))
    {
        qWarning() << "Incoming contact request has an invalid signature; rejecting";
        sendResponse(0x81);
        return;
    }

    /* Either a nickname or a message must be sent */
    if (nickname.isEmpty() && message.isEmpty())
    {
        qWarning() << "Incoming contact request has neither a nickname nor a message; rejecting";
        sendResponse(0x82);
        return;
    }

    /* Request is valid; the hidden service identity is cryptographically proven. */
    qDebug() << "Received contact request:";
    qDebug() << "  Hostname:" << hostname;
    qDebug() << "  Connection Secret:" << connSecret.toHex();
    qDebug() << "  Nickname:" << nickname;
    qDebug() << "  Message:" << message;
    qDebug() << "  Cookie:" << cookie.toHex();

    contactsManager->incomingRequests->addRequest(hostname, connSecret, this, nickname, message);

    /* addRequest() can automatically accept or reject in certain situations; account for that */
    if (state == SentResponse)
        return;

    /* Acknowledgement */
    sendResponse(0x00);

    /* We are now waiting for acceptance from the user; connection is held open. */
    state = WaitResponse;
    return;
}
