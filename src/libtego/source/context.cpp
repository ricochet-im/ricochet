#include "context.hpp"

//
// Tego Context
//

tego_context::tego_context()
: callback_registry_(this)
, callback_queue_(this)
{
    logger::trace();
}
