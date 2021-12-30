#pragma once
// C

// libtego
#include <tego/tego.h>

// C++

// standard library
#include <stdexcept>
#include <memory>
#include <utility>
#include <memory>
#include <type_traits>

// libtego
#include <tego/utilities.hpp>
//#define ENABLE_TEGO_LOGGER
#include <tego/logger.hpp>

namespace tego
{
    //
    // converts tego_error_t** C style error handling to exceptions
    //
    class throw_on_error
    {
    public:
        ~throw_on_error() noexcept(false)
        {
            if (error_ != nullptr)
            {
                logger::println("exception thrown : {}", tego_error_get_message(error_));
                std::runtime_error ex(tego_error_get_message(error_));
                tego_error_delete(error_);
                error_ = nullptr;
                throw ex;
            }
        }

        operator tego_error_t**()
        {
            return &error_;
        }
    private:
        tego_error_t* error_ = nullptr;
    };

    //
    // to_string methods to convert various tego types to human readable strings
    //
    inline std::string to_string(tego_file_hash_t const* fileHash)
    {

        if (fileHash == nullptr) return {};

        // size of string including null terminator
        const auto hashSize = tego_file_hash_string_size(fileHash, tego::throw_on_error());

        // std::string expects length as arg, not buffer size
        std::string hashString(hashSize-1, 0);
        tego_file_hash_to_string(fileHash, const_cast<char*>(hashString.data()), hashSize, tego::throw_on_error());

        return hashString;
    }
}


// define deleters for using unique_ptr and shared_ptr with tego types

#define TEGO_DEFAULT_DELETE_IMPL(TYPE)\
namespace std {\
    template<> class default_delete<TYPE##_t> {\
    public:\
        void operator()(TYPE##_t* val) { TYPE##_delete(val); }\
    };\
}

TEGO_DEFAULT_DELETE_IMPL(tego_ed25519_private_key)
TEGO_DEFAULT_DELETE_IMPL(tego_ed25519_public_key)
TEGO_DEFAULT_DELETE_IMPL(tego_ed25519_signature)
TEGO_DEFAULT_DELETE_IMPL(tego_v3_onion_service_id)
TEGO_DEFAULT_DELETE_IMPL(tego_tor_launch_config)
TEGO_DEFAULT_DELETE_IMPL(tego_tor_daemon_config)
TEGO_DEFAULT_DELETE_IMPL(tego_user_id)
TEGO_DEFAULT_DELETE_IMPL(tego_file_hash)

