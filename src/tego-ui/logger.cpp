#include <QString>
#include <QByteArray>

#include "logger.hpp"

std::ostream& operator<<(std::ostream& out, const QString& str)
{
    auto utf8str = str.toUtf8();
    out << utf8str.constData();
    return out;
}

// hex dump QByteArray

std::ostream& operator<<(std::ostream& out, const QByteArray& blob)
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

// type name

std::ostream& operator<<(std::ostream& out, const std::type_info& ti)
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

