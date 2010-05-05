#ifndef OUTGOINGREQUESTMANAGER_H
#define OUTGOINGREQUESTMANAGER_H

#include <QObject>
#include <QMap>

class ContactsManager;
class ContactUser;
class ContactRequestClient;

class OutgoingRequestManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(OutgoingRequestManager)

public:
    ContactsManager * const contacts;

    explicit OutgoingRequestManager(ContactsManager *contactsManager);

    void loadRequests();
    void addNewRequest(ContactUser *user, const QString &myNickname, const QString &message);

private slots:
    void handleResponse(int response);

private:
    QMap<ContactUser*,ContactRequestClient*> users;

    void startRequest(ContactUser *user);
};

#endif // OUTGOINGREQUESTMANAGER_H
