#pragma once

static_assert(ED25519_SIG_LEN == TEGO_ED25519_SIGNATURE_LENGTH);

typedef struct tego_ed25519_private_key* tego_ed25519_private_key_t;
typedef struct tego_ed25519_public_key* tego_ed25519_public_key_t;
typedef struct tego_ed25519_signature* tego_ed25519_signature_t;

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