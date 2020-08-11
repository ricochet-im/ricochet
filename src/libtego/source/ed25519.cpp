#include "error.hpp"
#include "ed25519.hpp"

extern "C"
{
    void tego_ed25519_private_key_from_ed25519_keyblob(
        tego_ed25519_private_key_t* out_privateKey,
        const char* keyBlob,
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {

        }, error);
    }

    void tego_ed25519_public_key_from_ed25519_private_key(
        tego_ed25519_public_key_t* out_publicKey,
        const tego_ed25519_private_key_t privateKey,
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {

        }, error);
    }

    void tego_ed25519_public_key_from_v3_onion_domain(
        tego_ed25519_public_key_t* out_publicKey,
        const char* v3OnionDomain,
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {

        }, error);
    }

    void tego_ed25519_signature_from_data(
        tego_ed25519_signature_t* out_signature,
        const uint8_t data[TEGO_ED25519_SIGNATURE_LENGTH],
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {

        }, error);
    }

    void tego_message_ed25519_sign(
        const uint8_t* message,
        size_t messageLength,
        const tego_ed25519_public_key_t privateKey,
        const tego_ed25519_public_key_t publicKey,
        tego_ed25519_signature_t* out_signature,
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {

        }, error);
    }

    void tego_ed25519_signature_data(
        const tego_ed25519_signature_t signature,
        uint8_t out_data[TEGO_ED25519_SIGNATURE_LENGTH],
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {

        }, error);
    }

    int tego_ed25519_signature_verify(
        const tego_ed25519_signature_t signature,
        const uint8_t* message,
        size_t messageLength,
        const tego_ed25519_public_key_t publicKey,
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> int
        {
            return TEGO_FALSE;
        }, error, TEGO_FALSE);
    }
}