#pragma once

#include <cstddef>
#include <iostream>
#include <fstream>
#include <mutex>

#define FMT_HEADER_ONLY
#include "fmt/format.h"
#include "fmt/ostream.h"

class logger
{
public:
    template<size_t N, typename... ARGS>
    static void println(const char (&format)[N], ARGS&&... args)
    {
        std::lock_guard<std::mutex> guard(get_mutex());

        auto& fs = get_stream();

        fmt::print(fs, "[{}] ", get_timestamp());
        fmt::print(fs, format, std::forward<ARGS>(args)...);
        fs << std::endl;
    }

    template<size_t N>
    static void println(const char (&msg)[N])
    {
        std::lock_guard<std::mutex> guard(get_mutex());

        auto& fs = get_stream();

        fmt::print(fs, "[{}] ", get_timestamp());
        fs << msg << std::endl;
    }

private:
    static std::ofstream& get_stream()
    {
        static std::ofstream fs("/tmp/ricochet.log", std::ios::binary);
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

inline std::ostream& operator<<(std::ostream& out, const QString& str)
{
    auto utf8str = str.toUtf8();
    out << utf8str.constData();
    return out;
}
