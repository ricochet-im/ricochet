
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
}