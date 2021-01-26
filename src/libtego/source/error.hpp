#pragma once

#define TEGO_THROW_MSG(FMT, ...) throw std::runtime_error(fmt::format(FMT, __VA_ARGS__));
#define TEGO_THROW_IF_FALSE_MSG(B, MSG) if (!(B)) { throw std::runtime_error(fmt::format("assertion failed {}:{} : {}", __FILE__, __LINE__, MSG)); }
#define TEGO_THROW_IF_FALSE(B) TEGO_THROW_IF_FALSE_MSG(B, #B)
#define TEGO_THROW_IF_TRUE_MSG(B, MSG) if (B) { throw std::runtime_error(fmt::format("assertion failed {}:{} : {}", __FILE__, __LINE__, MSG)); }
#define TEGO_THROW_IF_TRUE(B) TEGO_THROW_IF_TRUE_MSG(B, #B)
#define TEGO_THROW_IF_NULL(PTR) TEGO_THROW_IF_FALSE_MSG((PTR != nullptr), #PTR " may not be null")
#define TEGO_THROW_IF_NOT_NULL(PTR) TEGO_THROW_IF_FALSE_MSG((PTR == nullptr), #PTR " must be null")

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