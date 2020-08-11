#include "ed25519.hpp"
#include "error.hpp"

extern "C"
{
    void tego_ed25519_private_key_delete(tego_ed25519_private_key_t* privateKey)
    {
        delete privateKey;
    }

    void tego_ed25519_public_key_delete(tego_ed25519_public_key_t* publicKey)
    {
        delete publicKey;
    }

    void tego_ed25519_signature_delete(tego_ed25519_signature_t* signature)
    {
        delete signature;
    }

    void tego_v3_onion_service_id_delete(tego_v3_onion_service_id_t* serviceId)
    {
        delete serviceId;
    }

    void tego_error_delete(tego_error_t* error)
    {
        delete error;
    }
}