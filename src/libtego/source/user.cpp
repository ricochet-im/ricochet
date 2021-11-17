#include "error.hpp"
#include "user.hpp"

tego_user_id::tego_user_id(const tego_v3_onion_service_id_t& onionServiceId)
: serviceId(onionServiceId)
{ }

extern "C"
{
    void tego_user_id_copy(
        tego_user_id_t const* userId,
        tego_user_id_t** out_userId,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(userId);
            TEGO_THROW_IF_NULL(out_userId);

            auto retval = std::make_unique<tego_user_id_t>(*userId);
            *out_userId = retval.release();
        }, error);
    }

    void tego_user_id_from_v3_onion_service_id(
        tego_user_id_t** out_userId,
        const tego_v3_onion_service_id_t* serviceId,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(out_userId);
            TEGO_THROW_IF_FALSE(*out_userId == nullptr);
            TEGO_THROW_IF_NULL(serviceId);

            auto userId = std::make_unique<tego_user_id>(*serviceId);
            *out_userId = userId.release();
        }, error);
    }

    void tego_user_id_get_v3_onion_service_id(
        const tego_user_id_t* userId,
        tego_v3_onion_service_id_t** out_serviceId,
        tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            TEGO_THROW_IF_NULL(userId);
            TEGO_THROW_IF_NULL(out_serviceId);
            TEGO_THROW_IF_FALSE(*out_serviceId == nullptr);

            auto serviceId = std::make_unique<tego_v3_onion_service_id>(userId->serviceId);
            *out_serviceId = serviceId.release();
        }, error);
    }
}
