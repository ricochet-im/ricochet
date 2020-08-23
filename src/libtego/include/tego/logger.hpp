#pragma once

// std
#include <memory>
#include <cstddef>
#include <iostream>
#include <fstream>
#include <mutex>
#include <typeinfo>
#include <experimental/source_location>
using std::experimental::source_location;
#include <thread>

// gnu
#include <cxxabi.h>

#include <fmt/format.h>
#include <fmt/ostream.h>

class logger
{
public:
    template<size_t N, typename... ARGS>
    static void println(const char (&format)[N], ARGS&&... args)
    {
        std::lock_guard<std::mutex> guard(get_mutex());

        auto& fs = get_stream();

        fmt::print(fs, "[{:f}][{}] ", get_timestamp(), std::this_thread::get_id());
        fmt::print(fs, format, std::forward<ARGS>(args)...);
        fs << std::endl;
    }

    template<size_t N>
    static void println(const char (&msg)[N])
    {
        std::lock_guard<std::mutex> guard(get_mutex());

        auto& fs = get_stream();

        fmt::print(fs, "[{:f}][{}] ", get_timestamp(), std::this_thread::get_id());
        fs << msg << std::endl;
    }

    static void trace(const source_location& loc = source_location::current());
private:
    static std::ofstream& get_stream();
    static std::mutex& get_mutex();
    static double get_timestamp();
};

std::ostream& operator<<(std::ostream& out, const class QString& str);
std::ostream& operator<<(std::ostream& out, const class QByteArray& blob);
std::ostream& operator<<(std::ostream& out, const std::type_info& ti);

template<typename T>
std::string type_name(T&&)
{
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res = {
        abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, &status),
        std::free
    };

    if (status == 0)
    {
        return std::string(res.get());
    }
    return std::string(typeid(T).name());
}