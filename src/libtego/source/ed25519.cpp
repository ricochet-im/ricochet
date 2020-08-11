#include "error.hpp"
#include "ed25519.hpp"
#include "utilities.hpp"

#define TEGO_ED25519_KEYBLOB_HEADER "ED25519-V3:"
constexpr size_t TEGO_ED25519_KEYBLOB_HEADER_LENGTH = tego::static_strlen(TEGO_ED25519_KEYBLOB_HEADER);
// number of bytes needed to encode 64 bit private key as base64 not including null terminator
// ceiling(64 bytes / 3) * 4
constexpr size_t TEGO_ED25519_KEYBLOB_BASE64_LENGTH = 88;

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

            // ensure KeyBlob starts with correct string constant
            TEGO_THROW_IF_FALSE(std::string_view(keyBlob).starts_with(TEGO_ED25519_KEYBLOB_HEADER));

            // get a string_view of the base64 blob
            std::string_view base64(keyBlob + TEGO_ED25519_KEYBLOB_HEADER_LENGTH);

            // make sure the blob has enough characters to encode our privat ekey
            const auto maxByteCount = ::base64_decode_maxsize(base64.size());
            TEGO_THROW_IF_FALSE(maxByteCount >= ED25519_SECKEY_LEN);

            // local buffer for private key
            uint8_t privateKeyData[ED25519_SECKEY_LEN] = {0};
            const auto bytesWritten = ::base64_decode(reinterpret_cast<char*>(privateKeyData), sizeof(privateKeyData), base64.data(), base64.size());
            TEGO_THROW_IF_FALSE(bytesWritten == ED25519_SECKEY_LEN);

            // copy into returned tego_ed25519_private_key struct
            auto privateKey = new tego_ed25519_private_key();
            std::copy(std::begin(privateKeyData), std::end(privateKeyData), privateKey->data);

            *out_privateKey = privateKey;
        }, error);
    }

    void tego_ed25519_keyblob_from_ed25519_private_key(
        char out_keyBlob[TEGO_ED25519_KEYBLOB_LENGTH],
        const tego_ed25519_private_key_t privateKey,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(out_keyBlob != nullptr);
            TEGO_THROW_IF_FALSE(privateKey != nullptr);

            // init KeyBlob buffer with the header
            char keyBlob[
                TEGO_ED25519_KEYBLOB_HEADER_LENGTH +
                TEGO_ED25519_KEYBLOB_BASE64_LENGTH + 1] = TEGO_ED25519_KEYBLOB_HEADER;

            // encode privatekey as base64
            auto base64BytesWritten = base64_encode(
                    keyBlob + TEGO_ED25519_KEYBLOB_HEADER_LENGTH,
                    TEGO_ED25519_KEYBLOB_BASE64_LENGTH + 1,
                    reinterpret_cast<const char*>(privateKey->data),
                    sizeof(privateKey->data),
                    0);

            TEGO_THROW_IF_FALSE(base64BytesWritten == TEGO_ED25519_KEYBLOB_BASE64_LENGTH);

            // copy entire KeyBlob to output buffer
            std::copy(std::begin(keyBlob), std::end(keyBlob), out_keyBlob);
        }, error);
    }

    void tego_ed25519_public_key_from_ed25519_private_key(
        tego_ed25519_public_key_t* out_publicKey,
        const tego_ed25519_private_key_t privateKey,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(out_publicKey != nullptr);
            TEGO_THROW_IF_FALSE(*out_publicKey == nullptr);
            TEGO_THROW_IF_FALSE(privateKey != nullptr);

            // local buffer for public key
            uint8_t publicKeyData[ED25519_PUBKEY_LEN] = {0};
            TEGO_THROW_IF_FALSE(ed25519_donna_pubkey(publicKeyData, privateKey->data) == 0);

            // copy into returned tego_ed25519_public_key struct
            auto publicKey = new tego_ed25519_public_key();
            std::copy(std::begin(publicKeyData), std::end(publicKeyData), publicKey->data);

            *out_publicKey = publicKey;
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