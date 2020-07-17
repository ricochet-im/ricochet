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

inline std::ostream& operator<<(std::ostream& out, const QByteArray& blob)
{
    constexpr size_t rowWidth = 32;
    const size_t rowCount = blob.size() / rowWidth;

    const char* head = blob.data();
    size_t address = 0;

    auto printRow = [&](size_t count) -> void
    {
        constexpr auto octetGrouping = 4;
        fmt::print(out, "{:08x} : ", address);
        for(size_t k = 0; k < count; k++)
        {
            if ((k % octetGrouping) == 0) {
                fmt::print(out, " ");
            }
            fmt::print(out, "{:02x}", (uint8_t)head[k]);
        }
        for(size_t k = count; k < rowWidth; k++)
        {
            if ((k % octetGrouping) == 0) {
                fmt::print(out, " ");
            }
            fmt::print(out, "..");
        }

        fmt::print(out, " | ");
        for(size_t k = 0; k < count; k++)
        {
            char c = head[k];
            if (std::isprint(c))
            {
                fmt::print(out, "{}", c);
            }
            else
            {
                fmt::print(out, ".");
            }
        }

        out << '\n';

        address += rowWidth;
        head += rowWidth;
    };

    // foreach row
    for(size_t i = 0; i < rowCount; i++)
    {
        printRow(rowWidth);

    }

    // remainder
    const size_t remainder = (blob.size() % rowWidth);
    if (remainder > 0)
    {
        printRow(remainder);
    }

    return out;
}

inline std::ostream& operator<<(std::ostream& out, const std::type_info& ti)
{
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res = {
        abi::__cxa_demangle(ti.name(), nullptr, nullptr, &status),
        std::free
    };

    if (status == 0)
    {
        out << res.get();
    }
    else
    {
        out << ti.name();
    }

    return out;
}