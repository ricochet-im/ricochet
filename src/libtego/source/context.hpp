#pragma once

#include "signals.hpp"
#include "tor.hpp"

#include "tor/TorControl.h"
#include "tor/TorManager.h"

//
// Tego Context
//

struct tego_context
{
public:
    tego_context();

    void start_tor(const tego_tor_launch_config_t* config);
    const char* get_tor_version_string() const;
    tego_tor_control_status_t get_tor_control_status() const;
    tego_tor_daemon_status_t get_tor_daemon_status() const;
    int32_t get_tor_bootstrap_progress() const;
    tego_tor_bootstrap_tag_t get_tor_bootstrap_tag() const;
    void update_tor_daemon_config(const tego_tor_daemon_config_t* config);
    void save_tor_daemon_config();

    tego::callback_registry callback_registry_;
    tego::callback_queue callback_queue_;
    // anything that touches internal state should do so through
    // this 'global' (actually per tego_context) mutex
    std::mutex mutex_;

    // TODO: figure out ownership of these Qt Tor types
    Tor::TorManager* torManager = nullptr;
    Tor::TorControl* torControl = nullptr;
    mutable std::string torVersion;
private:
};