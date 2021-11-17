#include "ContactsManager.h"
#include "UserIdentity.h"
#include "IncomingContactRequest.h"

shims::UserIdentity* shims::UserIdentity::userIdentity = nullptr;

namespace shims
{
    UserIdentity::UserIdentity(tego_context_t* context_)
    : contacts(context_)
    , context(context_)
    , online(false)

    { }

    void UserIdentity::createIncomingContactRequest(const QString& hostname, const QString& message)
    {
        auto incomingContactRequest = new shims::IncomingContactRequest(hostname, message);
        this->requests.push_back(incomingContactRequest);

        emit this->requestAdded(incomingContactRequest);
        emit this->requestsChanged();
    }

    void UserIdentity::removeIncomingContactRequest(shims::IncomingContactRequest* incomingContactRequest)
    {
        auto it = std::find(this->requests.begin(), this->requests.end(), incomingContactRequest);
        Q_ASSERT(it != this->requests.end());

        this->requests.erase(it);
        emit this->requestsChanged();

        incomingContactRequest->deleteLater();
    }

    QList<QObject*> UserIdentity::getRequests() const
    {
        logger::trace();
        QList<QObject*> retval;
        retval.reserve(requests.size());
        for(auto currentRequest: requests)
        {
            retval.push_back(currentRequest);
        }
        return retval;
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

    void UserIdentity::setOnline(bool isOnline)
    {
        this->online = isOnline;
        emit this->statusChanged();
    }
}
