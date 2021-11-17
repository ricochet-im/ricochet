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
            else
            {
                emit success(); // removes the popup when the id returns to being valid
            }
            return re;
        }

        if (!isValidID(text) || matchingContact(text) || matchesIdentity(text))
        {
            emit failed();
            return QValidator::Invalid;
        }

        emit success();
        return re;
    }

    shims::ContactUser* ContactIDValidator::matchingContact(const QString &text) const
    {
        logger::trace();
        logger::println("UserIdentity instance : {}", static_cast<void*>(UserIdentity::userIdentity));
        auto contactsManager = UserIdentity::userIdentity->getContacts();
        logger::println("ContactsManager : {}", static_cast<void*>(contactsManager));
        return contactsManager->getShimContactByContactId(text);
    }

    bool ContactIDValidator::matchesIdentity(const QString &text) const
    {
        logger::trace();
        logger::println("UserIdentity instance : {}", static_cast<void*>(UserIdentity::userIdentity));
        auto context = UserIdentity::userIdentity->getContext();
        logger::println("context : {}", static_cast<void*>(UserIdentity::userIdentity->getContext()));

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

    bool ContactIDValidator::isValidID(const QString &serviceID) const
    {
        auto strippedID = serviceID.mid(tego::static_strlen("ricochet:"));
        logger::println("strippedID : {}", strippedID.toUtf8().constData(), strippedID.size());

        bool valid = tego_v3_onion_service_id_string_is_valid(strippedID.toUtf8().constData(), static_cast<size_t>(strippedID.size()), nullptr) == TEGO_TRUE;
        logger::println("valid: {}", valid);

        return valid;
    }
}
