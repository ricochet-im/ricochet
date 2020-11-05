#include "error.hpp"
#include "context.hpp"
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

            auto context = std::make_unique<tego_context>();
            *out_context = context.release();
        }, error);
    }

    void tego_uninitialize(tego_context_t* context, tego_error_t** error)
    {
        return tego::translateExceptions([=]() -> void
        {
            if (context)
            {
                delete context;
            }
        }, error);
    }
}