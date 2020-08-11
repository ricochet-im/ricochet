// C headers

// tor
#define ALL_BUGS_ARE_FATAL
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#   include <src/lib/log/util_bug.h>
#   include <ext/ed25519/donna/ed25519_donna_tor.h>
#   include <src/lib/defs/x25519_sizes.h>
#   include <src/lib/encoding/binascii.h>
#ifdef __cplusplus
}

// include our public header
#include <tego/tego.h>
#include <tego/logger.hpp>

#endif // __cplusplus

// C++ headers
#ifdef __cplusplus

// stl
#include <string_view>
#include <cstdio>
#include <stdexcept>

// fmt
#include <fmt/format.h>
#include <fmt/ostream.h>

// Qt
#include <QString>
#include <QByteArray>

#endif //__cplusplus