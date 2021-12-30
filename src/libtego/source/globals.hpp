#pragma once

#include "context.hpp"

namespace tego
{
    // dumping ground for global flags and variables
    struct globals
    {
        globals() = default;

        bool opensslAllocatorInited = false;
        bool secureRNGSeeded = false;
        std::unique_ptr<tego_context> context = nullptr;

        static globals instance;
    };

    inline constexpr globals& g_globals = globals::instance;
}