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

    void addNewRequest(ContactUser *user, const QString &myNickname, const QString &message);

    void acceptRequest(ContactUser *user);
    void rejectRequest(ContactUser *user, const QString &reason);

public slots:
    void loadRequests();

private slots:
    void requestAccepted();
    void requestRejected(int reason);

private:
    QMap<ContactUser*,ContactRequestClient*> users;

    void startRequest(ContactUser *user);
    void removeRequest(ContactUser *user);
};

#endif // OUTGOINGREQUESTMANAGER_H
