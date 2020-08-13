#ifndef TEGO_H
#define TEGO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define TEGO_TRUE  1
#define TEGO_FALSE 0

// number of bytes in an ed25519 signature
#define TEGO_ED25519_SIGNATURE_SIZE 64
// length of a v3 service id string including null terminator
#define TEGO_V3_ONION_SERVICE_ID_SIZE 57
// length of an ed25519 keyblob string including null terminator
#define TEGO_ED25519_KEYBLOB_SIZE 100

typedef struct tego_error* tego_error_t;

/*
 * Get error message form tego_error
 *
 * @param error : the error object to get the message from
 * @return : null terminated string with error message whose
 *  lifetime is tied to the source tego_error_t
 */
const char* tego_error_get_message(const tego_error_t error);

// library init/uninit
void tego_initialize(tego_error_t* error);
void tego_uninitialize(tego_error_t* error);

/*
 * v3 onion/ed25519 functionality
 */

typedef struct tego_ed25519_private_key* tego_ed25519_private_key_t;
typedef struct tego_ed25519_public_key* tego_ed25519_public_key_t;
typedef struct tego_ed25519_signature* tego_ed25519_signature_t;
typedef struct tego_v3_onion_service_id* tego_v3_onion_service_id_t;

/*
 * Conversion method for converting the KeyBlob string returned by
 * ADD_ONION command into an ed25519_private_key_t
 *
 * @param out_privateKey : returned ed25519 private key
 * @param keyBlob : an ED25519 KeyBlob string in the form
 *  "ED25519-V3:abcd1234..."
 * @param keyBlobLength : number of characters in keyBlob not
 *  counting the null terminator
 * @param error : filled with a tego_error_t on error
 */
void tego_ed25519_private_key_from_ed25519_keyblob(
    tego_ed25519_private_key_t* out_privateKey,
    const char* keyBlob,
    size_t keyBlobLength,
    tego_error_t* error);

/*
 * Conversion method for converting an ed25519 private key
 * to a null-terminated KeyBlob string for use with ADD_ONION
 * command
 *
 * @param out_keyBlob : buffer to be filled with ed25519 KeyBlob in
 *  the form "ED25519-V3:abcd1234...\0"
 * @param keyBlobSize : size of out_keyBlob buffer in bytes, must be at
 *  least 100 characters (99 for string + 1 for null terminator)
 * @param privateKey : the private key to encode
 * @param error : filled with a tego_error_t on error
 * @return : the number of characters written (including null terminator)
 *  to out_keyBlob
 */
size_t tego_ed25519_keyblob_from_ed25519_private_key(
    char *out_keyBlob,
    size_t keyBlobSize,
    const tego_ed25519_private_key_t privateKey,
    tego_error_t* error);

/*
 * Calculate ed25519 public key from ed25519 private key
 *
 * @param out_publicKey : returned ed25519 public key
 * @param privateKey : input ed25519 private key
 * @param error : filled with a tego_error_t on error
 */
void tego_ed25519_public_key_from_ed25519_private_key(
    tego_ed25519_public_key_t* out_publicKey,
    const tego_ed25519_private_key_t privateKey,
    tego_error_t* error);

/*
 * Construct a service id object from string. Validates
 * the checksum and version byte per spec:
 * https://gitweb.torproject.org/torspec.git/tree/rend-spec-v3.txt
 *
 * @param out_serviceId : returned v3 onion service id
 * @param serviceIdString : a string beginning with a v3 service id
 * @param serviceIdStringLength : length of the service id string not
 *  counting the null terminator
 * @param error : filled with a tego_error_t on error
 */
void tego_v3_onion_service_id_from_string(
    tego_v3_onion_service_id_t* out_serviceId,
    const char* serviceIdString,
    size_t serviceIdStringLength,
    tego_error_t* error);

/*
 * Serializes out a service id object as a null terminated string
 * string to provided character buffer.
 *
 * @param serviceId : v3 onion service id object to serialize
 * @param out_serviceIdString : destination buffer for string
 * @param serviceIdStringSize : size of out_serviceIdString buffer in
 *  bytes, must be at least 57 bytes (56 bytes for string + null
 *  terminator)
 * @param error : filled with a tego_error_t on error
 */
size_t tego_v3_onion_service_id_to_string(
    const tego_v3_onion_service_id_t serviceId,
    char* out_serviceIdString,
    size_t serviceIdStringSize,
    tego_error_t* error);

/*
 * Extract public key from v3 service id per
 * https://gitweb.torproject.org/torspec.git/tree/rend-spec-v3.txt
 *
 * @param out_publicKey : returned ed25519 public key
 * @param serviceId : input service id
 * @param error : filled with a tego_error_t on error
 */
void tego_ed25519_public_key_from_v3_onion_service_id(
    tego_ed25519_public_key_t* out_publicKey,
    const tego_v3_onion_service_id_t serviceId,
    tego_error_t* error);

/*
 * Derive an onion's service id from its ed25519 public key per
 * https://gitweb.torproject.org/torspec.git/tree/rend-spec-v3.txt
 *
 * @param out_serviceId : returned service id
 * @param publicKey : the public key input
 * @param error : filled with a tego_error_t on error
 */
void tego_v3_onion_service_id_from_ed25519_public_key(
    tego_v3_onion_service_id_t* out_serviceId,
    const tego_ed25519_public_key_t publicKey,
    tego_error_t* error);

/*
 * Read in signature from length 64 byte buffer
 *
 * @param out_signature : returned ed25519 signature
 * @param data : source memory buffer holding signature
 * @param dataSize : size of data in bytes, must be at least 64 bytes
 * @param error : filled with a tego_error_t on error
 */
void tego_ed25519_signature_from_data(
    tego_ed25519_signature_t* out_signature,
    const uint8_t* data,
    size_t dataSize,
    tego_error_t* error);

/*
 * Get the signature and place it in length 64 byte buffer
 *
 * @param signature : a calculated message signature
 * @param out_data : output buffer to write signature to
 * @param dataSize : size of data in bytes, must be at least 64 bytes
 * @param error : filled with a tego_error_t on error
 * @return : number of bytes written to out_data
 */
size_t tego_ed25519_signature_to_data(
    const tego_ed25519_signature_t signature,
    uint8_t* out_data,
    size_t dataSize,
    tego_error_t* error);

/*
 * Sign a message with an ed25519 key-pair
 *
 * @param message : binary blob to sign
 * @param messageLength : length of blob in bytes
 * @param privateKey : the ed25519 private key
 * @param publicKey : the ed25519 public key
 * @param out_signature : the output signature
 * @param error : filled with a tego_error_t on error
 */
void tego_message_ed25519_sign(
    const uint8_t* message,
    size_t messageLength,
    const tego_ed25519_public_key_t privateKey,
    const tego_ed25519_public_key_t publicKey,
    tego_ed25519_signature_t* out_signature,
    tego_error_t* error);

/*
 * Verify a message's signature given a public key
 *
 * @param signature : the signature to verify
 * @param message : the message that was signed
 * @param messageLength : length of the message
 * @param publicKey : the public key to very the the signature against
 * @param error : filled with a tego_error_t on error
 * @return : TEGO_TRUE if signature is verified, TEGO_FALSE if it is not
 *  verified or if an error occurs
 */
int tego_ed25519_signature_verify(
    const tego_ed25519_signature_t signature,
    const uint8_t* message,
    size_t messageLength,
    const tego_ed25519_public_key_t publicKey,
    tego_error_t* error);

/*
 Destructors for various tego types
 */

void tego_ed25519_private_key_delete(tego_ed25519_private_key_t);
void tego_ed25519_public_key_delete(tego_ed25519_public_key_t);
void tego_ed25519_signature_delete(tego_ed25519_signature_t);
void tego_v3_onion_service_id_delete(tego_v3_onion_service_id_t);
void tego_error_delete(tego_error_t);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // TEGO_H