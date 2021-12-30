#include "ContactsManager.h"
#include "ContactUser.h"

namespace shims
{
    ContactsManager::ContactsManager(tego_context_t* context_)
    : context(context_)
    , contactsList({})
    { }

    shims::ContactUser* ContactsManager::createContactRequest(
            const QString &contactID,
            const QString &nickname,
            const QString &myNickname,
            const QString &message)
    {
        logger::println("{{ contactID : {}, nickname : {}, myNickname : {}, message : {} }}",
            contactID, nickname, myNickname, message);

        auto serviceId = contactID.mid(tego::static_strlen("ricochet:")).toUtf8();

        // check that the service id is valid before anything else
        if (tego_v3_onion_service_id_string_is_valid(serviceId.constData(), static_cast<size_t>(serviceId.size()), nullptr) != TEGO_TRUE)
        {
            return nullptr;
        }

        auto shimContact = this->addContact(serviceId, nickname);

        auto userId = shimContact->toTegoUserId();
        auto rawMessage = message.toUtf8();

        tego_context_send_chat_request(this->context, userId.get(), rawMessage.data(), static_cast<size_t>(rawMessage.size()), tego::throw_on_error());

        shimContact->setStatus(shims::ContactUser::RequestPending);

        return shimContact;
    }

    shims::ContactUser* ContactsManager::addContact(const QString& serviceId, const QString& nickname)
    {
        // creates a new contact from service id and nickname
        auto shimContact = new shims::ContactUser(serviceId, nickname);
        contactsList.push_back(shimContact);

        // remove our reference and ready for deleting when contactDeleted signal is fireds
        connect(shimContact, &shims::ContactUser::contactDeleted, [self=this](shims::ContactUser* user) -> void
        {
            // find the given user in our internal list and remove, mark for deletion
            auto it = std::find(self->contactsList.begin(), self->contactsList.end(), user);
            self->contactsList.erase(it);

            user->deleteLater();
        });

        emit this->contactAdded(shimContact);
        return shimContact;
    }

    shims::ContactUser* ContactsManager::getShimContactByContactId(const QString& contactId) const
    {
        logger::trace();
        for(auto& cu : contactsList)
        {
            logger::println("cu : {}", static_cast<void*>(cu));
            if (cu->getContactID() == contactId)
            {
                logger::trace();
                return cu;
            }
        }
        return nullptr;
    }

    const QList<shims::ContactUser*>& ContactsManager::contacts() const
    {
        return contactsList;
    }

    void ContactsManager::setUnreadCount(shims::ContactUser* user, int unreadCount)
    {
        emit this->unreadCountChanged(user, unreadCount);
    }

    void ContactsManager::setContactStatus(shims::ContactUser* user, int status)
    {
        emit this->contactStatusChanged(user, status);
    }
}
