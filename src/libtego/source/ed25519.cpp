#include "error.hpp"
#include "ed25519.hpp"
#include "utilities.hpp"

// header of ed25519 KeyBlob returned by ADD_ONION comand
#define TEGO_ED25519_KEYBLOB_HEADER "ED25519-V3:"
// length of ed25519 KeyBlob header string not including null terminator
constexpr size_t TEGO_ED25519_KEYBLOB_HEADER_LENGTH = tego::static_strlen(TEGO_ED25519_KEYBLOB_HEADER);
// number of bytes needed to encode 64 bit private key as base64 not including null terminator
// ceiling(64 bytes / 3) * 4
constexpr size_t TEGO_ED25519_KEYBLOB_BASE64_LENGTH = 88;
// length of a valid v3 service id string not including null terminator
constexpr size_t TEGO_V3_SERVICE_ID_LENGTH = 56;
// length of a valid v3 onion address string (including trailing .onion) not including null terminator
constexpr size_t TEGO_V3_ONION_ADDRESS_LENGTH = TEGO_V3_SERVICE_ID_LENGTH + tego::static_strlen(".onion");
// number of bytes the base32 encoded service id string decodes to
constexpr size_t TEGO_V3_SERVICE_ID_RAW_SIZE = 35;
// offset to public key in raw service id
constexpr size_t TEGO_V3_SERVICE_ID_PUBLIC_KEY_OFFSET = 0;
// length of public key in raw service id
constexpr size_t TEGO_V3_SERVICE_ID_PUBLIC_KEY_SIZE = 32;
// offset to checksum in raw service id
constexpr size_t TEGO_V3_SERVICE_ID_CHECKSUM_OFFSET = 32;
// length of checksum in raw service id
constexpr size_t TEGO_V3_SERVICE_ID_CHECKSUM_SIZE = 2;
// offset to version (which should be 0x03) in raw service id
constexpr size_t TEGO_V3_SERVICE_ID_VERSION_OFFSET = 34;
// prefix used when calculating service id checksum
#define TEGO_V3_SERVICE_ID_CHECKSUM_SRC_PREFIX ".onion checksum"
// length of service id prefix not including null terminator
constexpr size_t TEGO_V3_SERVICE_ID_CHECKSUM_SRC_PREFIX_LEGNTH = tego::static_strlen(TEGO_V3_SERVICE_ID_CHECKSUM_SRC_PREFIX);
// offset to the public key in the service id checksum source
constexpr size_t TEGO_V3_SERVICE_ID_CHECKSUM_SRC_PUBLIC_KEY_OFFSET = TEGO_V3_SERVICE_ID_CHECKSUM_SRC_PREFIX_LEGNTH;
// size of source data used to calculate the serice id checksum
constexpr size_t TEGO_V3_SERVICE_ID_CHECKSUM_SRC_SIZE = TEGO_V3_SERVICE_ID_CHECKSUM_SRC_PREFIX_LEGNTH + ED25519_PUBKEY_LEN + 1;
// offset for the version byte in the service id checksum source
constexpr size_t TEGO_V3_SERVICE_ID_CHECKSUM_SRC_VERSION_OFFSET = TEGO_V3_SERVICE_ID_CHECKSUM_SRC_SIZE - 1;



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
            auto privateKey = std::make_unique<tego_ed25519_private_key>();
            std::copy(std::begin(privateKeyData), std::end(privateKeyData), privateKey->data);

            *out_privateKey = privateKey.release();
        }, error);
    }

    void tego_ed25519_keyblob_from_ed25519_private_key(
        char out_keyBlob[TEGO_ED25519_KEYBLOB_NULL_LENGTH],
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
            auto publicKey = std::make_unique<tego_ed25519_public_key>();
            std::copy(std::begin(publicKeyData), std::end(publicKeyData), publicKey->data);

            *out_publicKey = publicKey.release();
        }, error);
    }

    void tego_ed25519_public_key_from_v3_onion_address(
        tego_ed25519_public_key_t* out_publicKey,
        const char* v3OnionAddress,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            TEGO_THROW_IF_FALSE(out_publicKey != nullptr);
            TEGO_THROW_IF_FALSE(*out_publicKey == nullptr);
            TEGO_THROW_IF_FALSE(v3OnionAddress != nullptr);

            // https://gitweb.torproject.org/torspec.git/tree/rend-spec-v3.txt#n2135
            std::string_view v3OnionAddressView(v3OnionAddress);
            TEGO_THROW_IF_FALSE(v3OnionAddressView.size() == TEGO_V3_ONION_ADDRESS_LENGTH);
            TEGO_THROW_IF_FALSE(v3OnionAddressView.ends_with(".onion"));

            std::string_view base32ServiceId(v3OnionAddress, TEGO_V3_SERVICE_ID_LENGTH);
            uint8_t rawServiceId[TEGO_V3_SERVICE_ID_RAW_SIZE] = {0};

            // base32 decode service id
            const auto bytesDecoded = ::base32_decode(
                reinterpret_cast<char*>(rawServiceId),
                TEGO_V3_SERVICE_ID_RAW_SIZE,
                base32ServiceId.data(),
                base32ServiceId.size());
            TEGO_THROW_IF_FALSE(bytesDecoded == sizeof(rawServiceId));

            // verify correct version byte
            TEGO_THROW_IF_FALSE(rawServiceId[TEGO_V3_SERVICE_ID_VERSION_OFFSET] == 0x03);

            // build message for checksum
            // prefix
            uint8_t checksumSrc[TEGO_V3_SERVICE_ID_CHECKSUM_SRC_SIZE] = ".onion checksum";
            // public key
            std::copy(
                std::begin(rawServiceId),
                std::begin(rawServiceId) + ED25519_PUBKEY_LEN,
                checksumSrc + TEGO_V3_SERVICE_ID_CHECKSUM_SRC_PUBLIC_KEY_OFFSET);
            // version byte 0x03
            checksumSrc[TEGO_V3_SERVICE_ID_CHECKSUM_SRC_VERSION_OFFSET] = 0x03;

            // verify checksum
            uint8_t checksum[BASE32_DIGEST_LEN] = {0};

            // calculate sha256
            // TODO: probably just call openssl APIs directly here rather than use tor's
            // encapsulation
            TEGO_THROW_IF_FALSE(crypto_digest256(
                reinterpret_cast<char*>(checksum),
                reinterpret_cast<const char*>(checksumSrc),
                sizeof(checksumSrc),
                DIGEST_SHA3_256) == 0);

            // verify the first two bytes of checksum in service id match our calculated checksum
            TEGO_THROW_IF_FALSE(rawServiceId[TEGO_V3_SERVICE_ID_CHECKSUM_OFFSET    ] == checksum[0] &&
                                rawServiceId[TEGO_V3_SERVICE_ID_CHECKSUM_OFFSET + 1] == checksum[1]);

            // copy over public key
            auto publicKey = std::make_unique<tego_ed25519_public_key>();
            std::copy(std::begin(rawServiceId),
                      std::begin(rawServiceId) + ED25519_PUBKEY_LEN,
                      publicKey->data);

            *out_publicKey = publicKey.release();
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