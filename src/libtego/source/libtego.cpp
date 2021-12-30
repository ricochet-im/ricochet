#include "error.hpp"
#include "context.hpp"
#include "globals.hpp"
using namespace tego;

#include "utils/SecureRNG.h"

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

            // initialize OpenSSL's allocator
            if (!g_globals.opensslAllocatorInited) {
            #if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
                CRYPTO_malloc_init();
            #else
                OPENSSL_malloc_init();
            #endif

                g_globals.opensslAllocatorInited = true;
            }

            // seed our secure RNG
            if (!g_globals.secureRNGSeeded)
            {
                TEGO_THROW_IF_FALSE_MSG(SecureRNG::seed(), "Failed to initialize RNG");
                g_globals.secureRNGSeeded = true;
            }

            // create and save off singleton context
            g_globals.context = std::make_unique<tego_context>();
            *out_context = g_globals.context.get();

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