#include "context.hpp"
#include "error.hpp"
#include "globals.hpp"

//
// Tego Context
//

tego_context::tego_context()
: callback_registry_(this)
, callback_queue_(this)
{

}

void tego_context::start_tor(const tego_tor_configuration* config)
{

}