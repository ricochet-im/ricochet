#pragma once

#include "signals.hpp"

struct tego_context
{
public:
    tego_context();

    tego::callback_registry callback_registry_;
    tego::callback_queue callback_queue_;
    // anythign that touches internal state should do so through
    // this 'global' (actually per tego_context) mutex
    std::mutex mutex_;
private:
};

extern tego_context_t* g_tego_context;