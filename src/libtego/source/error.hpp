#pragma once

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
                *out_error = new tego_error{ex.what()};
            }
        }
        return onErrorReturn;
    }
}