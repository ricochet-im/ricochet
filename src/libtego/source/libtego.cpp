#include "error.hpp"

extern "C"
{
    void tego_initialize(tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {

        }, error);
    }

    void tego_uninitialize(tego_error_t* error)
    {
        return tego::translateExceptions([]() -> void
        {

        }, error);
    }
}