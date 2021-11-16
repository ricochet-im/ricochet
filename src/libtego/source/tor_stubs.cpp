#include "error.hpp"

#define NOT_USED(...) TEGO_THROW_MSG("{} should never be called", __FUNCTION__)

extern "C"
{
    void *tor_malloc_(size_t size)
    {
        return std::malloc(size);
    }

    void *tor_malloc_zero_(size_t size)
    {
        void* retval = tor_malloc_(size);
        if (retval) {
            std::memset(retval, 0x00, size);
        }
        return retval;
    }

    // convert assert to thrown exception
    void tor_assertion_failed_(
        const char *fname,
        unsigned int line,
        const char *func,
        const char *expr,
        const char *fmt,
        ...)
    {
        (void)func;
        (void)fmt;
        throw std::runtime_error(
            fmt::format("tor assertion failed {}:{} : {}", fname, line, expr));
    }

    // no-op, called as part of tor_assertion macro which
    // ultimately goes to tor_assertion_failed which throws an exception
    void tor_abort_(void)
    {
        throw std::runtime_error(__FUNCTION__);
    }

    // no-op swallow logging calls
    void log_fn_(int severity, log_domain_mask_t domain, const char *fn,
        const char *format, ...)
    {
        (void)severity;
        (void)domain;
        (void)fn;
        (void)format;
    }

#ifdef _WIN32
    const char* tor_fix_source_file(const char* fname)
    {
	   return fname;
    }
#endif

    void crypto_strongest_rand(uint8_t*, size_t)
    {
        NOT_USED();
    }

    void memwipe(void*, uint8_t, size_t)
    {
        NOT_USED();
    }

    size_t crypto_digest_algorithm_get_length(digest_algorithm_t)
    {
        NOT_USED();
        return {};
    }

    void* tor_memdup_(const void*, size_t)
    {
        NOT_USED();
        return {};
    }
}
