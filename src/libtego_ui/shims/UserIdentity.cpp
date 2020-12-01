#include "core/IncomingRequestManager.h"
#include "core/IdentityManager.h"
#include "core/UserIdentity.h"

#include "ContactsManager.h"
#include "UserIdentity.h"

shims::UserIdentity* shims::UserIdentity::userIdentity = nullptr;

namespace shims
{
    UserIdentity::UserIdentity(tego_context_t* context)
    : contacts(context)
    , context(context)
    , online(false)

    {
        // wire up slots to forward
        Q_ASSERT(identityManager->identities().size() == 1);
        auto userIdentity = identityManager->identities().first();

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
        tego_context_get_host_user_state(this->context, &state, tego::throw_on_error());

        return state == tego_host_user_state_online;
    }

    QString UserIdentity::contactID() const
    {
        // get host user id and convert to the ricochet:blahlah format
        std::unique_ptr<tego_user_id_t> userId;
        tego_context_get_host_user_id(this->context, tego::out(userId), tego::throw_on_error());

        std::unique_ptr<tego_v3_onion_service_id_t> serviceId;
        tego_user_id_get_v3_onion_service_id(userId.get(), tego::out(serviceId), tego::throw_on_error());

        char serviceIdString[TEGO_V3_ONION_SERVICE_ID_SIZE] = {0};
        tego_v3_onion_service_id_to_string(serviceId.get(), serviceIdString, sizeof(serviceIdString), tego::throw_on_error());

        QString contactId;
        QTextStream(&contactId) << "ricochet:" << serviceIdString;

        return contactId;
    }

    shims::ContactsManager* UserIdentity::getContacts()
    {
        logger::trace();
        return &contacts;
    }

    void UserIdentity::setOnline(bool online)
    {
        this->online = online;
        emit this->statusChanged();
    }
}