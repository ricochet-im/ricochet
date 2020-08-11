#pragma once

#define TEGO_THROW_IF_FALSE_MSG(B, MSG) if (!(B)) { throw std::runtime_error(fmt::format("assertion failed {}:{} : {}", __FILE__, __LINE__, MSG)); }
#define TEGO_THROW_IF_FALSE(B) TEGO_THROW_IF_FALSE_MSG(B, #B)

struct tego_error
{
    std::string message;
};

namespace tego
{
    auto translateExceptions(auto&& fn, tego_error_t* out_error) noexcept(true) -> void
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

    auto translateExceptions(auto&& fn, tego_error_t* out_error, decltype(fn()) onErrorReturn) noexcept(true) -> decltype(fn())
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