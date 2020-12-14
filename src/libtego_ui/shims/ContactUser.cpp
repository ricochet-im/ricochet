#include "core/ContactUser.h"

#include "ContactUser.h"
#include "ConversationModel.h"
#include "OutgoingContactRequest.h"

namespace shims
{
    ContactUser::ContactUser(tego_context_t* context, ::ContactUser* contactUser)
    : context(context)
    , contactUser(contactUser)
    , conversationModel(new shims::ConversationModel(this))
    , outgoingContactRequest(new shims::OutgoingContactRequest())
    {
        // wire up slots to forward
        connect(
            contactUser,
            &::ContactUser::nicknameChanged,
            [self=this]()
            {
                emit self->nicknameChanged();
            });

        connect(
            contactUser,
            &::ContactUser::statusChanged,
            [self=this]()
            {
                emit self->statusChanged();
            });

        connect(contactUser,
            &::ContactUser::contactDeleted,
            [self=this](::ContactUser*)
            {
                emit self->contactDeleted(self);
            });

        conversationModel->setContact(this);
    }

    QString ContactUser::getNickname() const
    {
        return contactUser->nickname();
    }

    QString ContactUser::getContactID() const
    {
        return contactUser->contactID();
    }

    ContactUser::Status ContactUser::getStatus() const
    {
        return static_cast<ContactUser::Status>(contactUser->status());
    }

    shims::OutgoingContactRequest* ContactUser::contactRequest()
    {
        outgoingContactRequest->setOutgoingContactRequest(contactUser->contactRequest());
        return outgoingContactRequest;
    }

    shims::ConversationModel* ContactUser::conversation()
    {
        return conversationModel;
    }

    void ContactUser::setNickname(const QString &nickname)
    {
        contactUser->setNickname(nickname);
    }

    void ContactUser::deleteContact()
    {
        contactUser->deleteContact();
    }
}