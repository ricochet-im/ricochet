#pragma once

#define TEGO_STRINGIFY_IMPL(X) #X
#define TEGO_STRINGIFY(X) TEGO_STRINGIFY_IMPL(X)

#define TEGO_THROW_MSG(FMT, ...) throw std::runtime_error(fmt::format("runtime error " __FILE__ ":" TEGO_STRINGIFY(__LINE__) " " FMT __VA_OPT__(,) __VA_ARGS__));

#define TEGO_THROW_IF_FALSE_MSG(B, ...) if (!(B)) { TEGO_THROW_MSG(__VA_ARGS__); }
#define TEGO_THROW_IF_FALSE(B) TEGO_THROW_IF_FALSE_MSG(B, "{} must be true", TEGO_STRINGIFY(B))

#define TEGO_THROW_IF_TRUE_MSG(B, ...) if (B) { TEGO_THROW_MSG("{}", __VA_ARGS__); }
#define TEGO_THROW_IF_TRUE(B) TEGO_THROW_IF_TRUE_MSG(B, "{} must be false", TEGO_STRINGIFY(B))

#define TEGO_THROW_IF_NULL(PTR) TEGO_THROW_IF_FALSE_MSG((PTR != nullptr), "{} must not be null", TEGO_STRINGIFY(PTR))
#define TEGO_THROW_IF_NOT_NULL(PTR) TEGO_THROW_IF_FALSE_MSG((PTR == nullptr), "{} must be null", TEGO_STRINGIFY(PTR))

struct tego_error
{
    std::string message;
};

namespace tego
{
    template<typename FUNC>
    auto translateExceptions(FUNC&& fn, tego_error_t** out_error) noexcept(true) -> void
    {
        static_assert(std::is_same<void, decltype(fn())>::value);

        try
        {
            fn();
        }
        catch(const std::exception& ex)
        {
            if (out_error)
            {
                logger::println("Exception: {}", ex.what());
                *out_error = new tego_error{ex.what()};
            }
        }
    }

    template<typename FUNC>
    auto translateExceptions(FUNC&& fn, tego_error_t** out_error, decltype(fn()) onErrorReturn) noexcept(true) -> decltype(fn())
    {
        try
        {
            return fn();
        }
        catch(const std::exception& ex)
        {
            if (out_error)
            {
                logger::println("Exception: {}", ex.what());
                *out_error = new tego_error{ex.what()};
            }
        }
        return onErrorReturn;
    }
}