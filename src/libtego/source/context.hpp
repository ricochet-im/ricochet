#pragma once

#include "signals.hpp"
#include "tor.hpp"
#include "user.hpp"

#include "tor/TorControl.h"
#include "tor/TorManager.h"
#include "core/IdentityManager.h"

//
// Tego Context
//

struct tego_context
{
public:
    tego_context();

    void start_tor(const tego_tor_launch_config_t* config);
    bool get_tor_daemon_configured() const;
    size_t get_tor_logs_size() const;
    const std::vector<std::string>& get_tor_logs() const;
    const char* get_tor_version_string() const;
    tego_tor_control_status_t get_tor_control_status() const;
    tego_tor_process_status_t get_tor_process_status() const;
    tego_tor_network_status_t get_tor_network_status() const;
    int32_t get_tor_bootstrap_progress() const;
    tego_tor_bootstrap_tag_t get_tor_bootstrap_tag() const;
    void start_service(
        tego_ed25519_private_key_t const* hostPrivateKey,
        tego_user_id_t const* const* userBuffer,
        tego_user_type_t* const userTypeBuffer,
        size_t userCount);
    void start_service();
    void update_tor_daemon_config(const tego_tor_daemon_config_t* config);
    void save_tor_daemon_config();
    void set_host_user_state(tego_host_user_state_t state);
    std::unique_ptr<tego_user_id_t> get_host_user_id() const;
    tego_host_user_state_t get_host_user_state() const;
    void send_chat_request(
        const tego_user_id_t* user,
        const char* message,
        size_t messageLength);
    void acknowledge_chat_request(
        const tego_user_id_t* user,
        tego_chat_acknowledge_t response);
    tego_message_id_t send_message(
        const tego_user_id_t* user,
        const std::string& message);
    tego_user_type_t get_user_type(tego_user_id_t const* user) const;
    size_t get_user_count() const;
    std::vector<tego_user_id_t*> get_users() const;
    void forget_user(const tego_user_id_t* user);
    std::tuple<tego_file_transfer_id_t, std::unique_ptr<tego_file_hash_t>, tego_file_size_t> send_file_transfer_request(
        tego_user_id_t const* user,
        std::string const& filePath);
    void respond_file_transfer_request(
        tego_user_id_t const* user,
        tego_file_transfer_id_t fileTransfer,
        tego_file_transfer_response_t response,
        std::string const& destPath);
    void cancel_file_transfer_transfer(
        tego_user_id_t const* user,
        tego_file_transfer_id_t);

    tego::callback_registry callback_registry_;
    tego::callback_queue callback_queue_;
    // anything that touches internal state should do so through
    // this 'global' (actually per tego_context) mutex
    std::mutex mutex_;

    // TODO: figure out ownership of these Qt types
    Tor::TorManager* torManager = nullptr;
    Tor::TorControl* torControl = nullptr;
    IdentityManager* identityManager = nullptr;

    // we store the thread id that this context is associated with
    // calls which go into our qt internals must be called from the same
    // thread as the context was created on
    // (this is not entirely true, they must be called from the thread with the Qt
    // event loop, which in our case is the thread the context is created on)
    std::thread::id threadId;
private:
    class ContactUser* getContactUser(const tego_user_id_t*) const;

    mutable std::string torVersion;
    mutable std::vector<std::string> torLogs;
    tego_host_user_state_t hostUserState = tego_host_user_state_unknown;
};