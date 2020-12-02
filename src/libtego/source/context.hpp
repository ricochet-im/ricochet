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
        tego_user_type_t const* const* userTypeBuffer,
        size_t userCount);
    void update_tor_daemon_config(const tego_tor_daemon_config_t* config);
    void save_tor_daemon_config();
    void set_host_user_state(tego_host_user_state_t state);
    std::unique_ptr<tego_user_id_t> get_host_user_id() const;
    tego_host_user_state_t get_host_user_state() const;
    void send_chat_request(
        const tego_user_id_t* user,
        const char* message,
        size_t messageLength);
    tego_message_id_t send_message(
        const tego_user_id_t* user,
        const std::string& message);

    tego::callback_registry callback_registry_;
    tego::callback_queue callback_queue_;
    // anything that touches internal state should do so through
    // this 'global' (actually per tego_context) mutex
    std::mutex mutex_;

    // TODO: figure out ownership of these Qt types
    Tor::TorManager* torManager = nullptr;
    Tor::TorControl* torControl = nullptr;
    IdentityManager* identityManager = nullptr;

    mutable std::string torVersion;
private:
    class ContactUser* getContactUser(const tego_user_id_t*);


    mutable std::vector<std::string> torLogs;
    tego_host_user_state_t hostUserState = tego_host_user_state_unknown;
};