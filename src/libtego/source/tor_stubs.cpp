
extern "C"
{
    // convert assert to thrown exception
    void tor_assertion_failed_(
        const char *fname,
        unsigned int line,
        const char *func,
        const char *expr,
        const char *fmt,
        ...)
    {
        throw std::runtime_error(
            fmt::format("tor assertion failed {}:{} : {}", fname, line, expr));
    }

    // no-op, called as part of tor_assertion macro
    void tor_abort_(void)
    {

    }
}