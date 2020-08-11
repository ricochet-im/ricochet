#pragma once
// C
#include <tego/tego.h>

// C++
#include <stdexcept>

namespace tego
{
    class throw_on_error
    {
    public:
        ~throw_on_error() noexcept(false)
        {
            if (error_ != nullptr)
            {
                std::runtime_error ex(tego_error_get_message(error_));
                tego_error_delete(error_);
                throw ex;
            }
        }

        operator tego_error_t*()
        {
            return &error_;
        }
    private:
        tego_error_t error_ = nullptr;
    };

}