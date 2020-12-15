#include "UserIdentity.h"
#include "ContactIDValidator.h"

namespace shims
{
    ContactIDValidator::ContactIDValidator(QObject *parent)
    : QRegularExpressionValidator(parent)
    {
        QRegularExpressionValidator::setRegularExpression(QRegularExpression(QStringLiteral("ricochet:([a-z2-7]{56})")));
    }

    void ContactIDValidator::fixup(QString &text) const
    {
        logger::trace();
        text = text.trimmed().toLower();
    }

    QValidator::State ContactIDValidator::validate(QString &text, int &pos) const
    {
        logger::trace();
        fixup(text);

        QValidator::State re = QRegularExpressionValidator::validate(text, pos);
        if (re != QValidator::Acceptable)
        {
            if (re == QValidator::Invalid)
            {
                emit failed();
            }
            return re;
        }

        if (matchingContact(text) || matchesIdentity(text))
        {
            emit failed();
            return QValidator::Invalid;
        }

        return re;
    }

    shims::ContactUser* ContactIDValidator::matchingContact(const QString &text) const
    {
        logger::trace();
        logger::println("UserIdentity instance : {}", (void*)UserIdentity::userIdentity);
        auto contactsManager = UserIdentity::userIdentity->getContacts();
        logger::println("ContactsManager : {}", (void*)contactsManager);
        return contactsManager->getShimContactByContactId(text);
    }

    bool ContactIDValidator::matchesIdentity(const QString &text) const
    {
        logger::trace();
        logger::println("UserIdentity instance : {}", (void*)UserIdentity::userIdentity);
        auto context = UserIdentity::userIdentity->getContext();
        logger::println("context : {}", (void*)UserIdentity::userIdentity->getContext());

        std::unique_ptr<tego_user_id_t> userId;
        tego_context_get_host_user_id(context, tego::out(userId), tego::throw_on_error());

        std::unique_ptr<tego_v3_onion_service_id_t> serviceId;
        tego_user_id_get_v3_onion_service_id(userId.get(), tego::out(serviceId), tego::throw_on_error());

        char serviceIdString[TEGO_V3_ONION_SERVICE_ID_SIZE] = {0};
        tego_v3_onion_service_id_to_string(serviceId.get(), serviceIdString, sizeof(serviceIdString), tego::throw_on_error());

        auto utf8Text = text.mid(tego::static_strlen("ricochet:")).toUtf8();
        auto utf8ServiceId = QByteArray(serviceIdString, TEGO_V3_ONION_SERVICE_ID_LENGTH);

        return utf8Text == utf8ServiceId;
    }
}