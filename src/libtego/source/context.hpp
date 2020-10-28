#pragma once

#include "signals.hpp"

struct tego_context
{
public:
    tego_context();

    tego::callback_registry callback_registry_;
    tego::callback_queue callback_queue_;
    // anythign that touches internal state shoudl do so through
    // this 'global' (per context) mutex
    std::mutex mutex_;
private:
};