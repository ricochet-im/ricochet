#include "context.hpp"
#include "error.hpp"

//
// Tego Context
//

tego_context_t* g_tego_context = nullptr;

tego_context::tego_context()
: callback_registry_(this)
, callback_queue_(this)
{
    TEGO_THROW_IF_FALSE(g_tego_context == nullptr);
    g_tego_context = this;
}
