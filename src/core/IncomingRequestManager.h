#ifndef INCOMINGREQUESTMANAGER_H
#define INCOMINGREQUESTMANAGER_H

#include <QObject>
#include <QPointer>
#include "protocol/ContactRequestServer.h"

class IncomingRequestManager;
class ContactsManager;
class QDateTime;

class IncomingContactRequest : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IncomingContactRequest)

public:
    IncomingRequestManager * const manager;
    const QByteArray hostname;

    IncomingContactRequest(IncomingRequestManager *manager, const QByteArray &hostname,
                           ContactRequestServer *connection = 0);

    QString message() const { return m_message; }
    void setMessage(const QString &message);

    QString nickname() const { return m_nickname; }

    bool hasActiveConnection() const { return connection != 0; }
    void setConnection(ContactRequestServer *connection);

    QDateTime requestDate() const;
    QDateTime lastRequestDate() const;

    void load();
    void save();

public slots:
    void setNickname(const QString &nickname);

    void accept();
    void reject();

private:
    QPointer<ContactRequestServer> connection;
    QString m_message, m_nickname;

    void removeRequest();
};

class IncomingRequestManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(IncomingRequestManager)

    friend class IncomingContactRequest;

public:
    ContactsManager * const contacts;

    explicit IncomingRequestManager(ContactsManager *contactsManager);

    QList<IncomingContactRequest*> requests() const { return m_requests; }
    IncomingContactRequest *requestFromHostname(const QByteArray &hostname);

    void loadRequests();

    /* Input from ContactRequestServer */
    void addRequest(const QByteArray &hostname, ContactRequestServer *connection, const QString &nickname,
                    const QString &message);

    /* Blacklist a host for immediate rejection in the future */
    void addRejectedHost(const QByteArray &hostname);

signals:
    void requestAdded(IncomingContactRequest *request);
    void requestRemoved(IncomingContactRequest *request);

private:
    QList<IncomingContactRequest*> m_requests;

    void removeRequest(IncomingContactRequest *request);
};

#endif // INCOMINGREQUESTMANAGER_H
