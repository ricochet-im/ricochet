#pragma once

#include "ContactUser.h"

class ContactsManager;
namespace shims
{
    class ContactsManager : public QObject
    {
        Q_OBJECT
        Q_DISABLE_COPY(ContactsManager)
    public:
        ContactsManager(tego_context_t* context);

        Q_INVOKABLE shims::ContactUser* createContactRequest(
            const QString &contactID,
            const QString &nickname,
            const QString &myNickname,
            const QString &message);
        const QList<shims::ContactUser*>& contacts() const;
        shims::ContactUser* getShimContact(::ContactUser*) const;
        shims::ContactUser* getShimContactByContactId(const QString& contactId) const;

    signals:
        void contactAdded(shims::ContactUser *user);
        void unreadCountChanged(shims::ContactUser *user, int unreadCount);
        void contactStatusChanged(shims::ContactUser* user, int status);
    private:
        ::ContactsManager* contactsManager;

        tego_context_t* context;
        mutable QList<shims::ContactUser*> contactsList;
    };
}
