#include "ed25519.hpp"
#include "error.hpp"
#include "context.hpp"
#include "tor.hpp"
#include "file_hash.hpp"

extern "C"
{
    #define TEGO_DELETE_IMPL(TYPE)\
    void TYPE##_delete(TYPE##_t* obj)\
    {\
        delete obj;\
    }

    TEGO_DELETE_IMPL(tego_ed25519_private_key)
    TEGO_DELETE_IMPL(tego_ed25519_public_key)
    TEGO_DELETE_IMPL(tego_ed25519_signature)
    TEGO_DELETE_IMPL(tego_v3_onion_service_id)
    TEGO_DELETE_IMPL(tego_error)
    TEGO_DELETE_IMPL(tego_tor_launch_config)
    TEGO_DELETE_IMPL(tego_tor_daemon_config)
    TEGO_DELETE_IMPL(tego_user_id)
    TEGO_DELETE_IMPL(tego_file_hash)
}
