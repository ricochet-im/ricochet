#include "core/IdentityManager.h"
#include "core/ContactsManager.h"
#include "core/UserIdentity.h"
#include "core/ContactUser.h"

#include "ContactsManager.h"
#include "ContactUser.h"

namespace shims
{
    ContactsManager::ContactsManager(tego_context_t* context)
    : contactsManager(identityManager->identities().first()->getContacts())
    , context(context)
    , contactsList({})
    {
        // wire up slots to forward
        connect(
            contactsManager,
            &::ContactsManager::unreadCountChanged,
            [self=this](::ContactUser* user, int unreadCount)
            {
                logger::trace();
                emit self->unreadCountChanged(self->getShimContact(user), unreadCount);
            });

        connect(
            contactsManager,
            &::ContactsManager::contactStatusChanged,
            [self=this](::ContactUser* user, int status)
            {
                logger::trace();
                logger::println("status : {}", status);
                emit self->contactStatusChanged(self->getShimContact(user), status);
            });
    }

    shims::ContactUser* ContactsManager::createContactRequest(
            const QString &contactID,
            const QString &nickname,
            const QString &myNickname,
            const QString &message)
    {
        logger::trace();

        logger::println("{{ contactID : {}, nickname : {}, myNickname : {}, message : {} }}",
            contactID, nickname, myNickname, message);

        auto contactUser = contactsManager->createContactRequest(
            contactID,
            nickname,
            myNickname,
            message);

        return getShimContact(contactUser);
    }

    shims::ContactUser* ContactsManager::getShimContact(::ContactUser* contactUser) const
    {
        for(auto current : this->contactsList)
        {
            if (current->contactUser == contactUser)
            {
                return current;
            }
        }

        logger::println("creating shim contact for : {}, {}", (void*)contactUser, contactUser->hostname());
        auto retval = new shims::ContactUser(context, contactUser);
        contactsList.push_back(retval);

        return retval;
    }

    shims::ContactUser* ContactsManager::getShimContactByContactId(const QString& contactId) const
    {
        for(auto cu : this->contacts())
        {
            if (cu->contactID() == contactId)
            {
                logger::trace();
                return cu;
            }
        }
        return nullptr;
    }


    const QList<shims::ContactUser*>& ContactsManager::contacts() const
    {
        // populate our shim contacts
        auto& realContacts = contactsManager->contacts();
        for(auto c : realContacts)
        {
            getShimContact(c);
        }

        return contactsList;
    }
}