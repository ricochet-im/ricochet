#include "UserIdentity.h"
#include "ContactUser.h"
#include "ConversationModel.h"
#include "OutgoingContactRequest.h"

// TODO: wire up the slots in here, figure out how to properly wire up unread count, status change
// populating the contacts manager on boot, keeping libtego's internal contacts synced with frontend's
// put all of the SettingsObject stuff into one settings manager

namespace shims
{
    ContactUser::ContactUser(const QString& serviceId_, const QString& nickname_)
    : conversationModel(new shims::ConversationModel(this))
    , outgoingContactRequest(new shims::OutgoingContactRequest())
    , status(ContactUser::Offline)
    , serviceId(serviceId_)
    , nickname()
    , settings(QString("users.%1").arg(serviceId_))
    {
        Q_ASSERT(serviceId.size() == TEGO_V3_ONION_SERVICE_ID_LENGTH);
        conversationModel->setContact(this);


        this->setNickname(nickname_);
    }

    QString ContactUser::getNickname() const
    {
        return nickname;
    }

    QString ContactUser::getContactID() const
    {
        return QString("ricochet:") + serviceId;
    }

    ContactUser::Status ContactUser::getStatus() const
    {
        return status;
    }

    void ContactUser::setStatus(ContactUser::Status newStatus)
    {
        if (this->status != newStatus)
        {
            this->status = newStatus;
            switch(this->status)
            {
                case ContactUser::Online:
                case ContactUser::Offline:
                    settings.write("type", "allowed");
                    break;
                case ContactUser::RequestPending:
                    settings.write("type", "pending");
                    break;
                case ContactUser::RequestRejected:
                    settings.write("type", "rejected");
                    break;
                default:
                    break;
            }
            emit this->statusChanged();
        }
    }

    shims::OutgoingContactRequest* ContactUser::contactRequest()
    {
        return outgoingContactRequest;
    }

    shims::ConversationModel* ContactUser::conversation()
    {
        return conversationModel;
    }

    void ContactUser::setNickname(const QString& newNickname)
    {
        if (this->nickname != newNickname)
        {
            this->nickname = newNickname;
            settings.write("nickname", newNickname);
            emit this->nicknameChanged();
        }
    }

    void ContactUser::deleteContact()
    {
        auto userIdentity = shims::UserIdentity::userIdentity;

        auto context = userIdentity->getContext();
        auto userId = this->toTegoUserId();

        tego_context_forget_user(context, userId.get(), tego::throw_on_error());

        settings.undefine();
        emit this->contactDeleted(this);
    }

    void ContactUser::sendFile()
    {
        this->conversationModel->sendFile();
    }

    bool ContactUser::exportConversation()
    {
        return this->conversationModel->exportConversation();
    }

    std::unique_ptr<tego_user_id_t> ContactUser::toTegoUserId() const
    {
        logger::println("serviceId : {}", this->serviceId);

        auto serviceIdRaw = this->serviceId.toUtf8();

        // ensure valid service id
        std::unique_ptr<tego_v3_onion_service_id_t> onionServiceId;
        tego_v3_onion_service_id_from_string(tego::out(onionServiceId), serviceIdRaw.data(), static_cast<size_t>(serviceIdRaw.size()), tego::throw_on_error());

        logger::trace();

        // create user id object from service id
        std::unique_ptr<tego_user_id_t> userId;
        tego_user_id_from_v3_onion_service_id(tego::out(userId), onionServiceId.get(), tego::throw_on_error());

        return userId;
    }
}
