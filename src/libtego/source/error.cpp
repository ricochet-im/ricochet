#include "error.hpp"

extern "C"
{
    const char* tego_error_get_message(const tego_error_t* error)
    {
        return error->message.c_str();
    }
}