#include "error.hpp"
#include "context.hpp"
#include "globals.hpp"

//
// Exports
//
extern "C"
{
    void tego_initialize(tego_context_t** out_context, tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            logger::println("init");

            TEGO_THROW_IF_NULL(out_context);
            TEGO_THROW_IF_FALSE(tego::g_globals.context.get() == nullptr);

            // create and save off singleton context
            tego::g_globals.context = std::make_unique<tego_context>();
            *out_context = tego::g_globals.context.get();

        }, error);
    }

    void tego_uninitialize(tego_context_t* context, tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            if (context)
            {
                tego::g_globals.context.reset(nullptr);
            }
        }, error);
    }
}