#pragma once

#include "signals.hpp"

struct tego_context
{
public:
    tego_context();

    void start_tor(const tego_tor_configuration* config);

    tego::callback_registry callback_registry_;
    tego::callback_queue callback_queue_;
    // anything that touches internal state should do so through
    // this 'global' (actually per tego_context) mutex
    std::mutex mutex_;
private:
};