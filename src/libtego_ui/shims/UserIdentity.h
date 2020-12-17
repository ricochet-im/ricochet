#pragma once

#include "ContactsManager.h"
namespace shims
{
    class IncomingContactRequest;
    class UserIdentity : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(UserIdentity)

        // needed by createDialog("ContactRequestDialog.qml",...) in main.qml
        Q_PROPERTY(QList<QObject*> requests READ getRequests NOTIFY requestsChanged)
        // used in TorPreferences.qml
        Q_PROPERTY(bool isOnline READ isServiceOnline NOTIFY statusChanged)
        // this originally had a contactIDChanged signal
        Q_PROPERTY(QString contactID READ contactID CONSTANT)
        // needed in MainWindow.qml
        Q_PROPERTY(shims::ContactsManager *contacts READ getContacts CONSTANT)
    public:
        UserIdentity(tego_context_t* context);

        void createIncomingContactRequest(const QString& hostname, const QString& message);
        void removeIncomingContactRequest(shims::IncomingContactRequest* incomingContactRequest);
        QList<QObject*> getRequests() const;
        bool isServiceOnline() const;
        QString contactID() const;
        shims::ContactsManager* getContacts();

        void setOnline(bool);

        static shims::UserIdentity* userIdentity;
        shims::ContactsManager contacts;

        tego_context_t* getContext() { return context; }
    signals:
        void statusChanged();
        // used in main.qml
        void requestAdded(shims::IncomingContactRequest *request);
        void requestsChanged();
        // used in MainWindow.qml
        void unreadCountChanged(ContactUser *user, int unreadCount);
        void contactStatusChanged(ContactUser* user, int status);

    private:
        QList<shims::IncomingContactRequest*> requests;

        tego_context_t *context;
        bool online;
    };
}