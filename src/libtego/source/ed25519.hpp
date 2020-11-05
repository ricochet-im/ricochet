#pragma once

static_assert(ED25519_SIG_LEN == TEGO_ED25519_SIGNATURE_SIZE);

struct tego_ed25519_private_key
{
    uint8_t data[ED25519_SECKEY_LEN] = {0};
};

struct tego_ed25519_public_key
{
    uint8_t data[ED25519_PUBKEY_LEN] = {0};
};

struct tego_ed25519_signature
{
    uint8_t data[ED25519_SIG_LEN] = {0};
};

struct tego_v3_onion_service_id
{
    tego_v3_onion_service_id() = default;
    tego_v3_onion_service_id(const char* serviceIdString, size_t serviceIdStringLength);

    char data[TEGO_V3_ONION_SERVICE_ID_SIZE] = {0};
};
