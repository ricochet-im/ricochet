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

// gnu
#include <cxxabi.h>

#define FMT_HEADER_ONLY
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

        fmt::print(fs, "[{:f}] ", get_timestamp());
        fmt::print(fs, format, std::forward<ARGS>(args)...);
        fs << std::endl;
    }

    template<size_t N>
    static void println(const char (&msg)[N])
    {
        std::lock_guard<std::mutex> guard(get_mutex());

        auto& fs = get_stream();

        fmt::print(fs, "[{:f}] ", get_timestamp());
        fs << msg << std::endl;
    }

    static void trace(const source_location& loc = source_location::current())
    {
        println("{}({})", loc.file_name(), loc.line());
    }

private:
    static std::ofstream& get_stream()
    {
        static std::ofstream fs("ricochet.log", std::ios::binary);
        return fs;
    }

    static std::mutex& get_mutex()
    {
        static std::mutex m;
        return m;
    }

    static double get_timestamp()
    {
        const static auto start = std::chrono::system_clock::now();
        const auto now = std::chrono::system_clock::now();
        std::chrono::duration<double> duration(now - start);
        return duration.count();
    }
};

std::ostream& operator<<(std::ostream& out, const QString& str);
std::ostream& operator<<(std::ostream& out, const QByteArray& blob);
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