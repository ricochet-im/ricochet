#pragma once

namespace tego
{
    // size of character buffer no counting the null terminator
    template<size_t N>
    constexpr size_t static_strlen(const char (&)[N])
    {
        return N - 1;
    }
}