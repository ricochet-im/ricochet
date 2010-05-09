#ifndef CONTACTREQUESTCLIENT_H
#define CONTACTREQUESTCLIENT_H

#include <QObject>

class ContactUser;

class ContactRequestClient : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactRequestClient)

public:
    ContactUser * const user;

    enum Response
    {
        NoResponse,
        Acknowledged,
        Accepted,
        Rejected,
        Error
    };

    explicit ContactRequestClient(ContactUser *user);

    QString message() const { return m_message; }
    void setMessage(const QString &message);

    QString myNickname() const { return m_mynick; }
    void setMyNickname(const QString &nick);

    Response response() const { return m_response; }

public slots:
    void sendRequest();

signals:
    void responseChanged(int response);

private slots:
    void socketConnected();
    void socketReadable();

private:
    class QTcpSocket *socket;
    QString m_message, m_mynick;
    Response m_response;

    enum
    {
        NotConnected,
        WaitConnect,
        WaitCookie,
        WaitAck,
        WaitResponse
    } state;

    bool buildRequestData(QByteArray cookie);
    bool handleResponse();
};

#endif // CONTACTREQUESTCLIENT_H
