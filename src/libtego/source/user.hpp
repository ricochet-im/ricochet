#pragma once

#include "ed25519.hpp"

struct tego_user_id
{
    tego_user_id(const tego_v3_onion_service_id_t&);

    tego_v3_onion_service_id_t serviceId;
};