#include "core/IncomingRequestManager.h"
#include "core/IdentityManager.h"
#include "core/ContactsManager.h"
#include "core/UserIdentity.h"

#include "UserIdentity.h"

shims::UserIdentity* shims::UserIdentity::userIdentity = nullptr;

namespace shims
{
    UserIdentity::UserIdentity(tego_context_t* context)
    : context(context)
    {
        // wire up slots to forward
        Q_ASSERT(identityManager->identities().size() == 1);
        auto userIdentity = identityManager->identities().first();

        // connect(
        //     userIdentity,
        //     &::UserIdentity::statusChanged,
        //     [self=this]()
        //     {
        //         emit self->statusChanged();
        //     });

        connect(
            userIdentity->getContacts()->incomingRequestManager(),
            &::IncomingRequestManager::requestAdded,
            [self=this](IncomingContactRequest* request)
            {
                emit self->requestAdded(request);
            });

        connect(
            userIdentity->getContacts()->incomingRequestManager(),
            &::IncomingRequestManager::requestsChanged,
            [self=this]()
            {
                emit self->requestsChanged();
            });
    }

    QList<QObject*> UserIdentity::requestObjects() const
    {
        logger::trace();
        auto userIdentity = identityManager->identities().first();
        auto incomingRequestManager = userIdentity->getContacts()->incomingRequestManager();

        return incomingRequestManager->requestObjects();
    }

    bool UserIdentity::isServiceOnline() const
    {
        logger::trace();

        auto state = tego_host_user_state_unknown;
        tego_context_get_host_user_state(context, &state, tego::throw_on_error());

        return state == tego_host_user_state_online;
    }

    void UserIdentity::createContactRequest(
            const QString &contactID,
            const QString &nickname,
            const QString &myNickname,
            const QString &message)
    {
        logger::trace();
        auto userIdentity = identityManager->identities().first();
        auto contactsManager = userIdentity->getContacts();

        contactsManager->createContactRequest(
            contactID,
            nickname,
            myNickname,
            message);
    }

    QString UserIdentity::contactID() const
    {
        logger::trace();
        auto userIdentity = identityManager->identities().first();
        return userIdentity->contactID();
    }

    ContactsManager* UserIdentity::getContacts() const
    {
        logger::trace();
        auto userIdentity = identityManager->identities().first();
        return userIdentity->getContacts();
    }

    void UserIdentity::setOnline(bool online)
    {
        this->online = online;
        emit this->statusChanged();
    }
}