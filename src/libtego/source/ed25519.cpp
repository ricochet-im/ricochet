#include "error.hpp"
#include "ed25519.hpp"
#include "utilities.hpp"

extern "C"
{
    void tego_ed25519_private_key_from_ed25519_keyblob(
        tego_ed25519_private_key_t* out_privateKey,
        const char* keyBlob,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(out_privateKey != nullptr);
            TEGO_THROW_IF_FALSE(*out_privateKey == nullptr);
            TEGO_THROW_IF_FALSE(keyBlob != nullptr);

            TEGO_THROW_IF_FALSE(std::string_view(keyBlob).starts_with("ED25519-V3:"));
            std::string_view base64(keyBlob + tego::static_strlen("ED25519-V3:"));

            const auto maxByteCount = ::base64_decode_maxsize(base64.size());
            TEGO_THROW_IF_FALSE(maxByteCount >= ED25519_SECKEY_LEN);

            auto privateKey = new tego_ed25519_private_key();
            const auto bytesWritten = ::base64_decode(reinterpret_cast<char*>(privateKey->data), sizeof(privateKey->data), base64.data(), base64.size());

            TEGO_THROW_IF_FALSE(bytesWritten == ED25519_SECKEY_LEN);

            *out_privateKey = privateKey;
        }, error);
    }

    void tego_ed25519_public_key_from_ed25519_private_key(
        tego_ed25519_public_key_t* out_publicKey,
        const tego_ed25519_private_key_t privateKey,
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {
            // generate ed25519_public_key_t from ed25519_secret_key_t using ed25519_donna_pubkey
            // ed25519_donna_pubkey(out_publicKey, privateKey);
        }, error);
    }

    void tego_ed25519_public_key_from_v3_onion_domain(
        tego_ed25519_public_key_t* out_publicKey,
        const char* v3OnionDomain,
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {
            // https://gitweb.torproject.org/torspec.git/tree/rend-spec-v3.txt#n2135

            // strip off .onion suffix from onion domain

            // base32 decode onion domain

            // public key is first 32 bytes

        }, error);
    }

    void tego_ed25519_signature_from_data(
        tego_ed25519_signature_t* out_signature,
        const uint8_t data[TEGO_ED25519_SIGNATURE_LENGTH],
        tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {
            // basically passthrough to ed25519_donna_sign
            // ed25519_donna_sign(out_signature, message, messageLength, privateKey, publicKey);
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
        // basically passthrough to ed25519_donna_sign
        // ed25519_donna_sign(out_signature, message, messageLength, privateKey, publicKey);
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
            // basically passthrough to ed25519_donna_open
            // ed25519_donna_open(signature, message, messageLength, publicKey);
            return TEGO_FALSE;
        }, error, TEGO_FALSE);
    }
}