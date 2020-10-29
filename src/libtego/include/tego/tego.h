#ifndef TEGO_H
#define TEGO_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define TEGO_TRUE  1
#define TEGO_FALSE 0

#define TEGO_FLAG(N) (1 << N)

// number of bytes in an ed25519 signature
#define TEGO_ED25519_SIGNATURE_SIZE 64
// length of a valid v3 service id string not including null terminator
#define TEGO_V3_ONION_SERVICE_ID_LENGTH 56
// length of a v3 service id string including null terminator
#define TEGO_V3_ONION_SERVICE_ID_SIZE (TEGO_V3_ONION_SERVICE_ID_LENGTH + 1)
// length of the ed25519 KeyBlob string not including null terminator
#define TEGO_ED25519_KEYBLOB_LENGTH 99
// length of an ed25519 keyblob string including null terminator
#define TEGO_ED25519_KEYBLOB_SIZE (TEGO_ED25519_KEYBLOB_LENGTH + 1)

typedef struct tego_error tego_error_t;

/*
 * Get error message form tego_error
 *
 * @param error : the error object to get the message from
 * @return : null terminated string with error message whose
 *  lifetime is tied to the source tego_error_t
 */
const char* tego_error_get_message(const tego_error_t* error);

// library init/uninit

typedef struct tego_context tego_context_t;

void tego_initialize(
    tego_context_t** out_context,
    tego_error_t** error);

void tego_uninitialize(
    tego_context_t* context,
    tego_error_t** error);

/*
 * v3 onion/ed25519 functionality
 */

typedef struct tego_ed25519_private_key tego_ed25519_private_key_t;
typedef struct tego_ed25519_public_key tego_ed25519_public_key_t;
typedef struct tego_ed25519_signature tego_ed25519_signature_t;
typedef struct tego_v3_onion_service_id tego_v3_onion_service_id_t;

/*
 * Conversion method for converting the KeyBlob string returned by
 * ADD_ONION command into an ed25519_private_key_t
 *
 * @param out_privateKey : returned ed25519 private key
 * @param keyBlob : an ED25519 KeyBlob string in the form
 *  "ED25519-V3:abcd1234..."
 * @param keyBlobLength : number of characters in keyBlob not
 *  counting the null terminator
 * @param error : filled on error
 */
void tego_ed25519_private_key_from_ed25519_keyblob(
    tego_ed25519_private_key_t** out_privateKey,
    const char* keyBlob,
    size_t keyBlobLength,
    tego_error_t** error);

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
 * @param error : filled on error
 * @return : the number of characters written (including null terminator)
 *  to out_keyBlob
 */
size_t tego_ed25519_keyblob_from_ed25519_private_key(
    char *out_keyBlob,
    size_t keyBlobSize,
    const tego_ed25519_private_key_t* privateKey,
    tego_error_t** error);

/*
 * Calculate ed25519 public key from ed25519 private key
 *
 * @param out_publicKey : returned ed25519 public key
 * @param privateKey : input ed25519 private key
 * @param error : filled on error
 */
void tego_ed25519_public_key_from_ed25519_private_key(
    tego_ed25519_public_key_t** out_publicKey,
    const tego_ed25519_private_key_t* privateKey,
    tego_error_t** error);

/*
 * Construct a service id object from string. Validates
 * the checksum and version byte per spec:
 * https://gitweb.torproject.org/torspec.git/tree/rend-spec-v3.txt
 *
 * @param out_serviceId : returned v3 onion service id
 * @param serviceIdString : a string beginning with a v3 service id
 * @param serviceIdStringLength : length of the service id string not
 *  counting the null terminator
 * @param error : filled on error
 */
void tego_v3_onion_service_id_from_string(
    tego_v3_onion_service_id_t** out_serviceId,
    const char* serviceIdString,
    size_t serviceIdStringLength,
    tego_error_t** error);

/*
 * Serializes out a service id object as a null terminated string
 * string to provided character buffer.
 *
 * @param serviceId : v3 onion service id object to serialize
 * @param out_serviceIdString : destination buffer for string
 * @param serviceIdStringSize : size of out_serviceIdString buffer in
 *  bytes, must be at least 57 bytes (56 bytes for string + null
 *  terminator)
 * @param error : filled on error
 * @return : number of bytes written including null terminator;
 *  TEGO_V3_ONION_SERVICE_ID_SIZE (57) on success, 0 on failure
 */
size_t tego_v3_onion_service_id_to_string(
    const tego_v3_onion_service_id_t* serviceId,
    char* out_serviceIdString,
    size_t serviceIdStringSize,
    tego_error_t** error);

/*
 * Extract public key from v3 service id per
 * https://gitweb.torproject.org/torspec.git/tree/rend-spec-v3.txt
 *
 * @param out_publicKey : returned ed25519 public key
 * @param serviceId : input service id
 * @param error : filled on error
 */
void tego_ed25519_public_key_from_v3_onion_service_id(
    tego_ed25519_public_key_t** out_publicKey,
    const tego_v3_onion_service_id_t* serviceId,
    tego_error_t** error);

/*
 * Derive an onion's service id from its ed25519 public key per
 * https://gitweb.torproject.org/torspec.git/tree/rend-spec-v3.txt
 *
 * @param out_serviceId : returned service id
 * @param publicKey : the public key input
 * @param error : filled on error
 */
void tego_v3_onion_service_id_from_ed25519_public_key(
    tego_v3_onion_service_id_t** out_serviceId,
    const tego_ed25519_public_key_t* publicKey,
    tego_error_t** error);

/*
 * Read in signature from length 64 byte buffer
 *
 * @param out_signature : returned ed25519 signature
 * @param buffer : source memory buffer holding signature
 * @param bufferSize : size of data in bytes, must be at least 64 bytes
 * @param error : filled on error
 */
void tego_ed25519_signature_from_bytes(
    tego_ed25519_signature_t** out_signature,
    const uint8_t* buffer,
    size_t bufferSize,
    tego_error_t** error);

/*
 * Get the signature and place it in length 64 byte buffer
 *
 * @param signature : a calculated message signature
 * @param out_buffer : output buffer to write signature to
 * @param bufferSize : size of buffer in bytes, must be at least 64 bytes
 * @param error : filled on error
 * @return : number of bytes written to out_buffer
 */
size_t tego_ed25519_signature_to_bytes(
    const tego_ed25519_signature_t* signature,
    uint8_t* out_buffer,
    size_t bufferSize,
    tego_error_t** error);

/*
 * Sign a message with an ed25519 key-pair
 *
 * @param message : message binary to sign
 * @param messageSize : size of message in bytes
 * @param privateKey : the ed25519 private key
 * @param publicKey : the ed25519 public key
 * @param out_signature : the output signature
 * @param error : filled on error
 */
void tego_message_ed25519_sign(
    const uint8_t* message,
    size_t messageSize,
    const tego_ed25519_private_key_t* privateKey,
    const tego_ed25519_public_key_t* publicKey,
    tego_ed25519_signature_t** out_signature,
    tego_error_t** error);

/*
 * Verify a message's signature given a public key
 *
 * @param signature : the signature to verify
 * @param message : the message that was signed
 * @param messageSize : size of the message in bytes
 * @param publicKey : the public key to very the the signature against
 * @param error : filled on error
 * @return : TEGO_TRUE if signature is verified, TEGO_FALSE if it is not
 *  verified or if an error occurs
 */
int tego_ed25519_signature_verify(
    const tego_ed25519_signature_t* signature,
    const uint8_t* message,
    size_t messageLength,
    const tego_ed25519_public_key_t* publicKey,
    tego_error_t** error);

/*
 * Chat protocol functionality
 */

/*
 * a unique identifier for a user, currently a v3 onion service id internally
 */
typedef struct tego_user_id tego_user_id_t;

// user id
/*
 * Convert a v3 onion service id to a user id
 *
 * @param out_userId : returned user id
 * @param serviceId : input v3 onion service id
 * @param error : filled on error
 */
void tego_user_id_from_v3_onion_service_id(
    tego_user_id_t** out_userId,
    const tego_v3_onion_service_id_t* serviceId,
    tego_error_t** error);

/*
 * Get the v3 onion service id from the user id
 *
 * @param userId : input user id
 * @param out_serviceId : returned v3 onion service id
 * @param error : filled on error
 */
void tego_user_id_get_v3_onion_service_id(
    const tego_user_id_t* userId,
    tego_v3_onion_service_id_t** out_serviceId,
    tego_error_t** error);

// TODO: figure out which statuses we need later
typedef enum
{
    tego_user_status_none    = 0,
    tego_user_status_online  = TEGO_FLAG(0),
    tego_user_status_offline = TEGO_FLAG(1),
} tego_user_status_t;

//
// contacts/user methods
//

/*
 * Get a user's current user status
 *
 * @param context : the current tego context
 * @param user : the user whose status we want
 * @param out_status : returned user status
 * @param error : filled on error
 */
void tego_context_get_user_status(
    const tego_context_t* context,
    const tego_user_id_t* user,
    tego_user_status_t* out_status,
    tego_error_t** error);

// enum for user type
typedef enum
{
    tego_user_type_allowed, // in our contact list
    tego_user_type_blocked, // in our block list
    tego_user_type_pending, // users the host has added but who have not replied yet
    tego_user_type_requesting, // users who have added host but the host has not replied yet
} tego_user_type_t;

/*
 * Get the number of users managed by our tego context
 *
 * @param context : the current tego context
 * @param userType : the type of user we want to count
 * @param out_userCount : gets the number of users with the given type
 * @param error : filled on error
 */
void tego_context_get_user_count(
    const tego_context_t* context,
    tego_user_type_t userType,
    size_t* out_userCount,
    tego_error_t** error);

/*
 * Get all of our users of a given type
 *
 * @param context : the current tego context
 * @param userType : the user type we want to filter by
 * @param out_usersBuffer : destination buffer to store returned user id pointers
 * @param usersBufferLength : maximum nuber of users that can be written to
 *  out_usersBuffer
 * @param out_usersCount : destination to store number of user ids written
 * @param error : filled on error
 */
void tego_context_get_users(
    const tego_context_t* context,
    tego_user_type_t userType,
    tego_user_id_t** out_usersBuffer,
    size_t usersBufferLength,
    size_t* out_usersCount,
    tego_error_t** error);

// struct that will contain configuration information for running the tor daemon
typedef struct tego_tor_configuration tego_tor_configuration_t;

//
// tor functions
//

// start tor daemon with given tor configuration
/*
 * Start an instance of the tor daemon and associate it with the given context
 *
 * @param context : the current tego context
 * @param torConfig : tor configuration params
 * @param error : filled on error
 */
void tego_context_start_tor(
    tego_context_t* context,
    const tego_tor_configuration* torConfig,
    tego_error_t** error);

/*
 * Update the tor daemon settings of running instance of tor associated
 * with a given tego context
 *
 * @param context : the current tego context
 * @param torConfig : tor configuration params
 * @param error : filled on error
 */
void tego_context_update_tor_configuration(
    tego_context_t* context,
    const tego_tor_configuration* torConfig,
    tego_error_t** error);

/*
 * Stops tor daemon associated with a given tego context
 *
 * @param context : the current tego context
 * @param error: filled on error
 */
void tego_context_stop_tor(
    tego_context_t* context,
    tego_error_t** error);

/*
 * Start tego's onion service and try to connect to users
 *
 * @param context : the current tego context
 * @param hostPrivateKey : the hosts private ed25519 key, or null if
 *  we want to create a new identity
 * @param userBuffer : the list of all users we care about
 * @param userTypeBuffer : the types associated with all of our users
 * @param userCount : the length of the user and user type buffers
 * @param error : filled on error
 */
void tego_context_start_service(
    tego_context_t* context,
    const tego_ed25519_private_key_t* hostPrivateKey,
    const tego_user_id_t** userBuffer,
    const tego_user_type_t** userTypeBuffer,
    size_t userCount,
    tego_error_t** error);

/*
 * Get the host's private key
 *
 * @param context : the current tego context
 * @param out_hostPrivateKey : the returned private key
 * @param error : filled on error
 */
void tego_context_get_host_private_key(
    tego_context_t* context,
    tego_ed25519_private_key_t** out_hostPrivateKey,
    tego_error_t** error);

/*
 * Stop tego's onion service associated with the given context
 *
 * @param context : the current tego context
 * @param error : filled on error */
void tego_context_stop_service(
    tego_context_t* context,
    tego_error_t** error);

/*
 * Fill the passed in buffer with the tor daemon's logs, each entry delimitted
 * by newline character '\n'
 *
 * @param context : the current tego context
 * @param out_logBuffer : user allocated buffer where tor log is to be written
 * @param logBufferSize : the size of the passed in out_logBuffer buffer
 * @param error : filled on error
 * @return : the nuber of characters written (including null terminator) to
 *  out_logBuffer
 */
size_t tego_context_get_tor_logs(
    const tego_context_t* context,
    char* out_logBuffer,
    size_t logBufferSize,
    tego_error_t** error);

/*
 * Get the null-terminated tor version string
 *
 * @param context : the curent tego context
 * @param error : filled on error
 * @return : the versio string for the context's running tor daemon
 */
const char* tego_context_get_tor_version_string(
    const tego_context_t* context,
    tego_error_t** error);

// various tor state flags currently exposed in ricochet ui
typedef enum
{
    tego_tor_state_none                 = 0,
    tego_tor_state_running              = TEGO_FLAG(0),
    tego_tor_state_control_connected    = TEGO_FLAG(1),
    tego_tor_state_circuits_established = TEGO_FLAG(2),
    tego_tor_state_onion_service_online = TEGO_FLAG(3),
} tego_tor_state_flags_t;

/*
 * Get current tor state flags
 *
 * @param context : the current tego context
 * @param out_stateFlags : destination to save flags
 * @param error : filled on error
 */
void tego_context_get_tor_state_flags(
    const tego_context_t* context,
    tego_tor_state_flags_t* out_stateFlags,
    tego_error_t** error);

//
// tego chat methods
//

/*
 * Send a text message from the host to the given user
 *
 * @param context : the current tego context
 * @param user : the user to send a message to
 * @param mesage : utf8 text message to send
 * @param messageLength : length of message including null-terminator
 * @param error : filled on error
 */
void tego_context_send_message(
    tego_context_t* context,
    tego_user_id_t* user,
    const char* message,
    size_t messageLength,
    tego_error_t** error);

/*
 * Adds the user to our allow list and enables them to connect to the host and
 * send messages. One would call this in response to a tego_chat_request_received_callback_t
 *
 * @param context : the current tego context
 * @param user : the user we want to allow
 * @param error : filled on error
 */
void tego_context_confirm_chat_request(
    tego_context_t* context,
    const tego_user_id_t* user,
    tego_error_t** error);

/*
 * Sends a request to chat to a user
 *
 * @param context : the current tego context
 * @param user : the user we want to chat with
 * @param error : filled on error
 */
void tego_context_send_chat_request(
    tego_context_t* context,
    const tego_user_id_t* user,
    tego_error_t** error);

/*
 * Prevent the given user from message the host
 *
 * @param context : the current tego context
 * @param user : the user to block
 * @param error : filled on error
 */
void tego_context_block_user(
    tego_context_t* context,
    const tego_user_id_t* user,
    tego_error_t** error);

/*
 * Forget about a given user, said user will be removed
 * from all internal lists and will be needed to be re-added
 * to chat
 *
 * @param context : the current tego context
 * @param user : the user to forget
 * @param error : filled on error
 */

void tego_context_forget_user(
    tego_context_t* context,
    const tego_user_id_t* user,
    tego_error_t* error);

//
// callbacks for frontend to respond to events
// provides no guarantees on what thread they are running on or thread safety
//

/*
 * Callback fired when the one of the tor state flags changes
 *
 * @param context : the current tego context
 * @param stateFlogs : the current state flags for our context's tor instance
 */
typedef void (*tego_tor_state_changed_callback_t)(
    tego_context_t* context,
    tego_tor_state_flags_t stateFlags);

// fired when a log from tor daemon arrives

/*
 * Callback fired when a log entry is received from the tor daemon
 *
 * @param context : the current tego context
 * @param message : a null-terminated log entry string
 * @param messageLength : length of the message including null-terminator
 */
typedef void (*tego_tor_log_received_callback_t)(
    tego_context_t* context,
    const char* message,
    size_t messageLength);

/*
 * Callback fired when the host receives a chat request from another user
 *
 * @param context : the current tego context
 * @param sender : the user that wants to chat
 * @param message : null-terminated message string received from the requesting user
 * @param messageLength : length of the message including null-terminator
 */
typedef void (*tego_chat_request_received_callback_t)(
    tego_context_t* context,
    const tego_user_id_t* sender,
    const char* message,
    size_t messageLength);

/*
 * Callback received when the host receives a message from another user
 *
 * @param context : the current tego context
 * @param sender : the user that sent host the message
 * @param message : null-terminated message string
 * @param messageLength : length of the message including null-terminator
 */
typedef void (*tego_message_received_callback_t)(
    tego_context_t* context,
    const tego_user_id_t* sender,
    const char* message,
    size_t messageLength);

/*
 * Callback fired when a user's status changes
 *
 * @param context : the current tego context
 * @param user : the user whose status has changed
 * @param status: the user's new status
 */
typedef void (*tego_user_status_changed_callback_t)(
    tego_context_t* context,
    const tego_user_id_t* user,
    tego_user_status_t status);

/*
 * Callback fired when tor creates a new onion service for
 * the host
 *
 * @param context : the current tego context
 * @param privateKey : the host's private key
 */
typedef void (*tego_new_identity_created_callback_t)(
    tego_context_t* context,
    const tego_ed25519_private_key_t* privateKey);

/*
 * Setters for various callbacks
 */

void tego_context_set_tor_state_changed_callback(
    tego_context_t* context,
    tego_tor_state_changed_callback_t,
    tego_error_t** error);

void tego_context_set_tor_log_received_callback(
    tego_context_t* context,
    tego_tor_log_received_callback_t,
    tego_error_t** error);

void tego_context_set_chat_request_received_callback(
    tego_context_t* context,
    tego_chat_request_received_callback_t,
    tego_error_t** error);

void tego_context_set_message_received_callback(
    tego_context_t* context,
    tego_message_received_callback_t,
    tego_error_t** error);

void tego_context_set_user_status_changed_callback(
    tego_context_t* context,
    tego_user_status_changed_callback_t,
    tego_error_t** error);

void tego_context_set_new_identity_created_callback(
    tego_context_t* context,
    tego_new_identity_created_callback_t,
    tego_error_t** error);


/*
 Destructors for various tego types
 */

// error
void tego_error_delete(tego_error_t*);

// crypto
void tego_ed25519_private_key_delete(tego_ed25519_private_key_t*);
void tego_ed25519_public_key_delete(tego_ed25519_public_key_t*);
void tego_ed25519_signature_delete(tego_ed25519_signature_t*);
void tego_v3_onion_service_id_delete(tego_v3_onion_service_id_t*);

// user
void tego_user_id_delete(tego_user_id_t*);

// tor
void tego_tor_configuration_delete(tego_tor_configuration_t*);


#ifdef __cplusplus
}
#endif // __cplusplus
#endif // TEGO_H