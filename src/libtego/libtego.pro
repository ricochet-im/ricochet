QMAKE_INCLUDES = $${PWD}/../qmake_includes

include($${QMAKE_INCLUDES}/artifacts.pri)
include($${QMAKE_INCLUDES}/compiler_flags.pri)

TEMPLATE = lib
TARGET = tego

CONFIG += staticlib

# setup precompiled headers
CONFIG += precompile_header
PRECOMPILED_HEADER = source/precomp.h

# fmt sources
FMT_SOURCE_DIR = $${PWD}/../extern/fmt/src
FMT_INCLUDE_DIR = $${PWD}/../extern/fmt/include
SOURCES +=\
    $${FMT_SOURCE_DIR}/format.cc\
    $${FMT_SOURCE_DIR}/os.cc

INCLUDEPATH +=\
    $${FMT_INCLUDE_DIR}

# tor sources
TOR_ROOT_DIR = $${PWD}/../extern/tor
TOR_SOURCE_DIR = $${TOR_ROOT_DIR}/src

SOURCES +=\
    $${TOR_SOURCE_DIR}/ext/ed25519/donna/ed25519_tor.c\
    $${TOR_SOURCE_DIR}/lib/encoding/binascii.c\
    $${TOR_SOURCE_DIR}/lib/crypt_ops/crypto_digest_openssl.c

INCLUDEPATH +=\
    $${TOR_ROOT_DIR}\
    $${TOR_ROOT_DIR}/src\
    $${TOR_ROOT_DIR}/src/ext

# libtego
HEADERS +=\
    include/tego/tego.h\
    include/tego/logger.hpp\
    include/tego/utilities.hpp\
    source/orconfig.h\
    source/error.hpp\
    source/ed25519.hpp

SOURCES +=\
    source/libtego.cpp\
    source/delete.cpp\
    source/error.cpp\
    source/tor_stubs.cpp\
    source/ed25519.cpp\
    source/logger.cpp\


# external
INCLUDEPATH +=\
    include\
    source\
    $${TOR_INCLUDE_DIR}

include($${QMAKE_INCLUDES}/openssl.pri)