#pragma once

namespace tego
{
    template<size_t N>
    constexpr size_t static_strlen(const char (&)[N])
    {
        return N - 1;
    }
}