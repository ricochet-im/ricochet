#include "error.hpp"
#include "ed25519.hpp"
#include "utilities.hpp"

// header of ed25519 KeyBlob returned by ADD_ONION comand
#define TEGO_ED25519_KEYBLOB_HEADER "ED25519-V3:"
// length of ed25519 KeyBlob header string not including null terminator
constexpr size_t TEGO_ED25519_KEYBLOB_HEADER_LENGTH = tego::static_strlen(TEGO_ED25519_KEYBLOB_HEADER);
// length of the ed25519 KeyBlob string not including null terminator
constexpr size_t TEGO_ED25519_KEYBLOB_LENGTH = TEGO_ED25519_KEYBLOB_SIZE - 1;
// length of ed25519 KeyBlob string not including null terminator
constexpr size_t TEGO_ED25519_KEYBLOB_BASE64_LENGTH = 88;
// number of bytes needed to encode KeyBlob including the null terminator
constexpr size_t TEGO_ED25519_KEYBLOB_BASE64_SIZE = TEGO_ED25519_KEYBLOB_BASE64_LENGTH + 1;
// length of a valid v3 service id string not including null terminator
constexpr size_t TEGO_V3_ONION_SERVICE_ID_LENGTH = 56;
// number of bytes the base32 encoded service id string decodes to
constexpr size_t TEGO_V3_ONION_SERVICE_ID_RAW_SIZE = 35;
// offset to public key in raw service id
constexpr size_t TEGO_V3_ONION_SERVICE_ID_PUBLIC_KEY_OFFSET = 0;
// length of public key in raw service id
constexpr size_t TEGO_V3_ONION_SERVICE_ID_PUBLIC_KEY_SIZE = 32;
// offset to checksum in raw service id
constexpr size_t TEGO_V3_ONION_SERVICE_ID_CHECKSUM_OFFSET = 32;
// length of checksum in raw service id
constexpr size_t TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SIZE = 2;
// offset to version (which should be 0x03) in raw service id
constexpr size_t TEGO_V3_ONION_SERVICE_ID_VERSION_OFFSET = 34;
// prefix used when calculating service id checksum
#define TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_PREFIX ".onion checksum"
// length of service id prefix not including null terminator
constexpr size_t TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_PREFIX_LEGNTH = tego::static_strlen(TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_PREFIX);
// offset to the public key in the service id checksum source
constexpr size_t TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_PUBLIC_KEY_OFFSET = TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_PREFIX_LEGNTH;
// size of source data used to calculate the serice id checksum
constexpr size_t TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_SIZE = TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_PREFIX_LEGNTH + ED25519_PUBKEY_LEN + 1;
// offset for the version byte in the service id checksum source
constexpr size_t TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_VERSION_OFFSET = TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_SIZE - 1;

namespace tego
{
    void truncated_checksum_from_ed25519_public_key(
        uint8_t out_truncatedChecksum[TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SIZE],
        const uint8_t (&publicKey)[ED25519_PUBKEY_LEN])
    {
        // build message for checksum
        // prefix
        uint8_t checksumSrc[TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_SIZE] = TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_PREFIX;
        // public key
        std::copy(
            std::begin(publicKey),
            std::begin(publicKey) + sizeof(publicKey),
            checksumSrc + TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_PUBLIC_KEY_OFFSET);
        // version byte 0x03
        checksumSrc[TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SRC_VERSION_OFFSET] = 0x03;

        // verify checksum
        uint8_t checksum[BASE32_DIGEST_LEN] = {0};

        // calculate sha256
        // TODO: probably just call openssl APIs directly here rather than use tor's
        // encapsulation to simplify build and include weirdness in precomp.h
        TEGO_THROW_IF_FALSE(crypto_digest256(
            reinterpret_cast<char*>(checksum),
            reinterpret_cast<const char*>(checksumSrc),
            sizeof(checksumSrc),
            DIGEST_SHA3_256) == 0);

        out_truncatedChecksum[0] = checksum[0];
        out_truncatedChecksum[1] = checksum[1];
    }
}

extern "C"
{
    void tego_ed25519_private_key_from_ed25519_keyblob(
        tego_ed25519_private_key_t* out_privateKey,
        const char* keyBlob,
        size_t keyBlobLength,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(out_privateKey != nullptr);
            TEGO_THROW_IF_FALSE(*out_privateKey == nullptr);
            TEGO_THROW_IF_FALSE(keyBlob != nullptr);
            TEGO_THROW_IF_FALSE(keyBlobLength == TEGO_ED25519_KEYBLOB_LENGTH);

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

    size_t tego_ed25519_keyblob_from_ed25519_private_key(
        char* out_keyBlob,
        size_t keyBlobSize,
        const tego_ed25519_private_key_t privateKey,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> size_t
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(out_keyBlob != nullptr);
            TEGO_THROW_IF_FALSE(keyBlobSize >= TEGO_ED25519_KEYBLOB_SIZE);
            TEGO_THROW_IF_FALSE(privateKey != nullptr);

            // zero out output buffer first
            std::fill(out_keyBlob, out_keyBlob + keyBlobSize, 0x00);

            // init KeyBlob buffer with the header
            char keyBlob[TEGO_ED25519_KEYBLOB_SIZE] = TEGO_ED25519_KEYBLOB_HEADER;

            // encode privatekey as base64 (adds null terminator)
            auto base64BytesWritten = base64_encode(
                keyBlob + TEGO_ED25519_KEYBLOB_HEADER_LENGTH,
                TEGO_ED25519_KEYBLOB_BASE64_SIZE,
                reinterpret_cast<const char*>(privateKey->data),
                sizeof(privateKey->data),
                0);

            TEGO_THROW_IF_FALSE(keyBlob[TEGO_ED25519_KEYBLOB_LENGTH] == 0);
            TEGO_THROW_IF_FALSE(base64BytesWritten == TEGO_ED25519_KEYBLOB_BASE64_LENGTH);

            // copy entire KeyBlob to output buffer
            std::copy(std::begin(keyBlob), std::end(keyBlob), out_keyBlob);
            return sizeof(keyBlob);
        }, error, 0);
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

    void tego_v3_onion_service_id_from_string(
        tego_v3_onion_service_id_t* out_serviceId,
        const char* serviceIdString,
        size_t serviceIdStringLength,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            TEGO_THROW_IF_FALSE(out_serviceId != nullptr);
            TEGO_THROW_IF_FALSE(*out_serviceId == nullptr);
            TEGO_THROW_IF_FALSE(serviceIdString != nullptr);
            TEGO_THROW_IF_FALSE(serviceIdStringLength >= TEGO_V3_ONION_SERVICE_ID_LENGTH);

            std::string_view serviceIdView(serviceIdString, TEGO_V3_ONION_SERVICE_ID_LENGTH);
            uint8_t rawServiceId[TEGO_V3_ONION_SERVICE_ID_RAW_SIZE] = {0};

            // base32 decode service id
            const auto bytesDecoded = ::base32_decode(
                reinterpret_cast<char*>(rawServiceId),
                sizeof(rawServiceId),
                serviceIdView.data(),
                serviceIdView.size());
            TEGO_THROW_IF_FALSE(bytesDecoded == sizeof(rawServiceId));

            // verify correct version byte
            TEGO_THROW_IF_FALSE(rawServiceId[TEGO_V3_ONION_SERVICE_ID_VERSION_OFFSET] == 0x03);

            // first part of the rawServiceId is the publicKey
            auto& rawPublicKey = reinterpret_cast<uint8_t (&)[ED25519_PUBKEY_LEN]>(rawServiceId);

            // calculate the truncated checksum for the public key
            uint8_t truncatedChecksum[TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SIZE] = {0};
            tego::truncated_checksum_from_ed25519_public_key(truncatedChecksum, rawPublicKey);

            // verify the first two bytes of checksum in service id match our calculated checksum
            TEGO_THROW_IF_FALSE(
                rawServiceId[TEGO_V3_ONION_SERVICE_ID_CHECKSUM_OFFSET    ] == truncatedChecksum[0] &&
                rawServiceId[TEGO_V3_ONION_SERVICE_ID_CHECKSUM_OFFSET + 1] == truncatedChecksum[1]);

            // verified checksum, copy service id string to new service id object
            auto serviceId = std::make_unique<tego_v3_onion_service_id>();
            std::copy(std::begin(serviceIdView), std::end(serviceIdView), serviceId->data);
            // write null terminator
            serviceId->data[TEGO_V3_ONION_SERVICE_ID_LENGTH] = 0;

            *out_serviceId = serviceId.release();
        }, error);
    }

    size_t tego_v3_onion_service_id_to_string(
        const tego_v3_onion_service_id_t serviceId,
        char* out_serviceIdString,
        size_t serviceIdStringSize,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> size_t
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(serviceId != nullptr);
            TEGO_THROW_IF_FALSE(out_serviceIdString != nullptr);
            TEGO_THROW_IF_FALSE(serviceIdStringSize >= TEGO_V3_ONION_SERVICE_ID_SIZE);

            std::copy(
                std::begin(serviceId->data),
                std::end(serviceId->data),
                out_serviceIdString);

            return TEGO_V3_ONION_SERVICE_ID_SIZE;
        }, error, 0);
    }

    void tego_ed25519_public_key_from_v3_onion_service_id(
        tego_ed25519_public_key_t* out_publicKey,
        const tego_v3_onion_service_id_t serviceId,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(out_publicKey != nullptr);
            TEGO_THROW_IF_FALSE(*out_publicKey == nullptr);
            TEGO_THROW_IF_FALSE(serviceId != nullptr);

            // https://gitweb.torproject.org/torspec.git/tree/rend-spec-v3.txt#n2135
            std::string_view serviceIdView(serviceId->data, TEGO_V3_ONION_SERVICE_ID_LENGTH);
            uint8_t rawServiceId[TEGO_V3_ONION_SERVICE_ID_RAW_SIZE] = {0};

            // base32 decode service id
            const auto bytesDecoded = ::base32_decode(
                reinterpret_cast<char*>(rawServiceId),
                sizeof(rawServiceId),
                serviceIdView.data(),
                serviceIdView.size());
            TEGO_THROW_IF_FALSE(bytesDecoded == sizeof(rawServiceId));

            // first part of the service id is the public key

            // copy over public key
            auto publicKey = std::make_unique<tego_ed25519_public_key>();
            std::copy(std::begin(rawServiceId),
                      std::begin(rawServiceId) + ED25519_PUBKEY_LEN,
                      publicKey->data);

            *out_publicKey = publicKey.release();
        }, error);
    }

    void tego_v3_onion_service_id_from_ed25519_public_key(
        tego_v3_onion_service_id_t* out_serviceId,
        const tego_ed25519_public_key_t publicKey,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(out_serviceId != nullptr);
            TEGO_THROW_IF_FALSE(*out_serviceId == nullptr);
            TEGO_THROW_IF_FALSE(publicKey != nullptr);

            // build the raw service id
            uint8_t rawServiceId[TEGO_V3_ONION_SERVICE_ID_RAW_SIZE] = {0};

            // copy over public key
            std::copy(std::begin(publicKey->data),
                      std::end(publicKey->data),
                      rawServiceId);

            // calculate turncated checksum and copy it into raw service id
            uint8_t truncatedChecksum[TEGO_V3_ONION_SERVICE_ID_CHECKSUM_SIZE] = {0};
            tego::truncated_checksum_from_ed25519_public_key(truncatedChecksum, publicKey->data);
            std::copy(std::begin(truncatedChecksum),
                      std::end(truncatedChecksum),
                      rawServiceId + TEGO_V3_ONION_SERVICE_ID_CHECKSUM_OFFSET);

            // version 3
            rawServiceId[TEGO_V3_ONION_SERVICE_ID_VERSION_OFFSET] = 0x03;

            // encode to base32
            char serviceIdString[TEGO_V3_ONION_SERVICE_ID_SIZE] = {0};
            ::base32_encode(serviceIdString, sizeof(serviceIdString), reinterpret_cast<const char*>(rawServiceId), sizeof(rawServiceId));

            auto serviceId = std::make_unique<tego_v3_onion_service_id>();
            std::copy(std::begin(serviceIdString), std::end(serviceIdString), serviceId->data);

            *out_serviceId = serviceId.release();
        }, error);
    }

    void tego_ed25519_signature_from_data(
        tego_ed25519_signature_t* out_signature,
        const uint8_t* data,
        size_t dataSize,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(out_signature != nullptr);
            TEGO_THROW_IF_FALSE(*out_signature == nullptr);
            TEGO_THROW_IF_FALSE(data != nullptr);
            TEGO_THROW_IF_FALSE(dataSize >= TEGO_ED25519_SIGNATURE_SIZE);

            // copy raw signature into signature struct
            auto signature = std::make_unique<tego_ed25519_signature>();
            std::copy(data, data + TEGO_ED25519_SIGNATURE_SIZE, signature->data);

            *out_signature = signature.release();

        }, error);
    }

    size_t tego_ed25519_signature_to_data(
        const tego_ed25519_signature_t signature,
        uint8_t* out_data,
        size_t dataSize,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> size_t
        {
            TEGO_THROW_IF_FALSE(signature != nullptr);
            TEGO_THROW_IF_FALSE(out_data != nullptr);
            TEGO_THROW_IF_FALSE(dataSize >= TEGO_ED25519_SIGNATURE_SIZE)

            // get the data out of our signature type
            std::copy(std::begin(signature->data), std::end(signature->data), out_data);
            return sizeof(signature->data);
        }, error, 0);
    }

    void tego_message_ed25519_sign(
        const uint8_t* message,
        size_t messageLength,
        const tego_ed25519_private_key_t privateKey,
        const tego_ed25519_public_key_t publicKey,
        tego_ed25519_signature_t* out_signature,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> void
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(message != nullptr);
            TEGO_THROW_IF_FALSE(messageLength > 0);
            TEGO_THROW_IF_FALSE(privateKey != nullptr);
            TEGO_THROW_IF_FALSE(publicKey != nullptr);
            TEGO_THROW_IF_FALSE(out_signature != nullptr);
            TEGO_THROW_IF_FALSE(*out_signature == nullptr);

            // calculate message signature
            uint8_t signatureBuffer[TEGO_ED25519_SIGNATURE_SIZE] = {0};
            TEGO_THROW_IF_FALSE(
                ::ed25519_donna_sign(
                    signatureBuffer,
                    message,
                    messageLength,
                    privateKey->data,
                    publicKey->data));

            auto signature = std::make_unique<tego_ed25519_signature>();
            std::copy(std::begin(signatureBuffer), std::end(signatureBuffer), signature->data);

            *out_signature = signature.release();
        }, error);
    }

    int tego_ed25519_signature_verify(
        const tego_ed25519_signature_t signature,
        const uint8_t* message,
        size_t messageLength,
        const tego_ed25519_public_key_t publicKey,
        tego_error_t* error)
    {
        return tego::translateExceptions([&]() -> int
        {
            // verify arguments
            TEGO_THROW_IF_FALSE(signature != nullptr);
            TEGO_THROW_IF_FALSE(message != nullptr);
            TEGO_THROW_IF_FALSE(messageLength > 0);
            TEGO_THROW_IF_FALSE(publicKey != nullptr);

            // attempt to verify
            auto result = ::ed25519_donna_open(
                signature->data,
                message,
                messageLength,
                publicKey->data);

            // result will be 0 if valid, -1 if not
            TEGO_THROW_IF_FALSE(result == 0 || result == -1);

            if (result == 0) return TEGO_TRUE;
            return TEGO_FALSE;
        }, error, TEGO_FALSE);
    }
}