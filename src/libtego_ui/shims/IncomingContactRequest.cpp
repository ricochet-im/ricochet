#include "IncomingContactRequest.h"
#include "UserIdentity.h"

namespace shims
{
    IncomingContactRequest::IncomingContactRequest(const QString& hostname, const QString& message)
    : serviceIdString(hostname.chopped(tego::static_strlen(".onion")))
    , nickname()
    , message(message)
    , userId()
    {
        auto serviceIdRaw = serviceIdString.toUtf8();

        std::unique_ptr<tego_v3_onion_service_id_t> serviceId;
        tego_v3_onion_service_id_from_string(tego::out(serviceId), serviceIdRaw.data(), serviceIdRaw.size(), tego::throw_on_error());

        tego_user_id_from_v3_onion_service_id(tego::out(userId), serviceId.get(), tego::throw_on_error());
    }

    QString IncomingContactRequest::getHostname() const
    {
        return serviceIdString + QString(".onion");
    }

    QString IncomingContactRequest::getContactId() const
    {
        return QString("ricochet:") + serviceIdString;
    }

    void IncomingContactRequest::setNickname(const QString& nickname)
    {
        logger::println("setNickname : '{}'", nickname);
        this->nickname = nickname;
        emit this->nicknameChanged();
    }

    void IncomingContactRequest::accept()
    {
        auto userIdentity = shims::UserIdentity::userIdentity;
        auto context = userIdentity->getContext();
        auto contactManager = userIdentity->getContacts();

        tego_context_acknowledge_chat_request(context, userId.get(), tego_chat_acknowledge_accept, tego::throw_on_error());

        userIdentity->removeIncomingContactRequest(this);

        contactManager->addContact(serviceIdString, nickname);
    }

    void IncomingContactRequest::reject()
    {
        auto userIdentity = shims::UserIdentity::userIdentity;
        auto context = userIdentity->getContext();

        tego_context_acknowledge_chat_request(context, userId.get(), tego_chat_acknowledge_block, tego::throw_on_error());

        userIdentity->removeIncomingContactRequest(this);
    }
}