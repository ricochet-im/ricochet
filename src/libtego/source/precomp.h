// C headers

// standard library
#include <stddef.h>

// tor
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
#   include <orconfig.h>
#   ifdef HAVE_EVP_SHA3_256
#       define OPENSSL_HAS_SHA3
#   endif // HAVE_EVP_SHA3_256
#   define ALL_BUGS_ARE_FATAL
#   include <src/lib/log/util_bug.h>
#   include <ext/ed25519/donna/ed25519_donna_tor.h>
#   include <src/lib/defs/x25519_sizes.h>
#   include <src/lib/encoding/binascii.h>
#   include <src/lib/crypt_ops/crypto_digest.h>
#ifdef __cplusplus
}

// include our public header
#include <tego/tego.h>
#include <tego/logger.hpp>

#endif // __cplusplus

// C++ headers
#ifdef __cplusplus

// standard library
#include <string_view>
#include <cstdio>
#include <stdexcept>
#include <memory>

// fmt
#include <fmt/format.h>
#include <fmt/ostream.h>

// Qt
#include <QString>
#include <QByteArray>

#endif //__cplusplus#i