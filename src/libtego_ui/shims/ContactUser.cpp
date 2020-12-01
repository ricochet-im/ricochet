#include "core/ContactUser.h"

#include "ContactUser.h"
#include "ConversationModel.h"

namespace shims
{
    ContactUser::ContactUser(tego_context_t* context, ::ContactUser* contactUser)
    : context(context)
    , contactUser(contactUser)
    , conversationModel(new shims::ConversationModel(this))
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

    OutgoingContactRequest* ContactUser::contactRequest()
    {
        return contactUser->contactRequest();
    }

    shims::ConversationModel* ContactUser::conversation()
    {
        return conversationModel;
    }

    void ContactUser::setNickname(const QString &nickname)
    {
        contactUser->setNickname(nickname);
    }
}