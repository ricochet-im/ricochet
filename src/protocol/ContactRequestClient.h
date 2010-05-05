#ifndef CONTACTREQUESTCLIENT_H
#define CONTACTREQUESTCLIENT_H

#include <QObject>
#include <QTcpSocket>

class ContactUser;

class ContactRequestClient : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactRequestClient)

public:
    ContactUser * const user;

    explicit ContactRequestClient(ContactUser *user);

    QString message() const { return m_message; }
    void setMessage(const QString &message);

    QString myNickname() const { return m_mynick; }
    void setMyNickname(const QString &nick);

public slots:
    void sendRequest();

private slots:
    void socketConnected();
    void socketReadable();

private:
    QTcpSocket socket;
    QString m_message, m_mynick;

    enum
    {
        NotConnected,
        WaitCookie,
        WaitAck,
        WaitResponse
    } state;

    bool buildRequestData(QByteArray cookie);
    bool handleResponse();
};

#endif // CONTACTREQUESTCLIENT_H
