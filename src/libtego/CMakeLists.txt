cmake_minimum_required(VERSION 3.16)

project(libtego LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (FORCE_QT5)
    find_package(
        QT
        NAMES
        Qt5
        COMPONENTS Core
                Gui
                Network
                Quick
                Widgets
        REQUIRED)
else ()
    find_package(
        QT
        NAMES
        Qt6
        Qt5
        COMPONENTS Core
                Gui
                Network
                Quick
                Widgets
        REQUIRED)
endif ()

find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS Core
               Widgets
               Network
               Qml
               Quick
    REQUIRED)

# Require Qt >5.15
if (${QT_VERSION_MAJOR} EQUAL 5)
    if (${QT_VERSION_MINOR} LESS 15)
        message(FATAL_ERROR "Qt >5.15 is required to build this project!")
    endif ()
endif ()

if (APPLE)
    find_package(
        Qt${QT_VERSION_MAJOR}
        COMPONENTS MacExtras
        REQUIRED)
endif ()

# protobuf configuration
set(Protobuf_USE_STATIC_LIBS OFF CACHE BOOL "Use statically-linked protobuf")
set(Protobuf_ROOT_DIR "" CACHE STRING "Path to custom Protobuf installation")
if (NOT ${Protobuf_ROOT_DIR} STREQUAL "")
    # CMake doesn't allow us to specify a protobuf installation like with
    # openssl. Instead, use CMAKE_PREFIX_PATH to tell CMake to check that dir
    # first for a protobuf install
    list(APPEND CMAKE_PREFIX_PATH ${Protobuf_ROOT_DIR}/lib/cmake/protobuf)
    if ( Protobuf_FOUND )
        message( STATUS "Protobuf version : ${Protobuf_VERSION}" )
        message( STATUS "Protobuf include path : ${Protobuf_INCLUDE_DIRS}" )
        message( STATUS "Protobuf libraries : ${Protobuf_LIBRARIES}" )
        message( STATUS "Protobuf compiler libraries : ${Protobuf_PROTOC_LIBRARIES}")
        message( STATUS "Protobuf lite libraries : ${Protobuf_LITE_LIBRARIES}")
    else()
        message( WARNING "Protobuf package not found -> specify search path via PROTOBUF_ROOT variable")
    endif()
endif ()
include(FindProtobuf)

# openssl configuration
set(OPENSSL_USE_STATIC_LIBS OFF CACHE BOOL "Use statically-linked OpenSSL")
set(OPENSSL_ROOT_DIR "" CACHE STRING "Path to custom OpenSSL installation")
include(FindOpenSSL)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(fmt REQUIRED)

add_library(
    tego STATIC
    include/tego/logger.hpp
    include/tego/tego.h
    include/tego/tego.hpp
    include/tego/utilities.hpp
    source/context.cpp
    source/context.hpp
    source/core/ContactIDValidator.cpp
    source/core/ContactIDValidator.h
    source/core/ContactUser.cpp
    source/core/ContactUser.h
    source/core/ContactsManager.cpp
    source/core/ContactsManager.h
    source/core/ConversationModel.cpp
    source/core/ConversationModel.h
    source/core/IdentityManager.cpp
    source/core/IdentityManager.h
    source/core/IncomingRequestManager.cpp
    source/core/IncomingRequestManager.h
    source/core/OutgoingContactRequest.cpp
    source/core/OutgoingContactRequest.h
    source/core/UserIdentity.cpp
    source/core/UserIdentity.h
    source/delete.cpp
    source/ed25519.cpp
    source/ed25519.hpp
    source/error.cpp
    source/error.hpp
    source/file_hash.cpp
    source/file_hash.hpp
    source/globals.cpp
    source/globals.hpp
    source/libtego.cpp
    source/logger.cpp
    source/orconfig.h
    source/precomp.h
    source/protocol/AuthHiddenServiceChannel.cpp
    source/protocol/AuthHiddenServiceChannel.h
    source/protocol/Channel.cpp
    source/protocol/Channel.h
    source/protocol/Channel_p.h
    source/protocol/ChatChannel.cpp
    source/protocol/ChatChannel.h
    source/protocol/Connection.cpp
    source/protocol/Connection.h
    source/protocol/Connection_p.h
    source/protocol/ContactRequestChannel.cpp
    source/protocol/ContactRequestChannel.h
    source/protocol/ControlChannel.cpp
    source/protocol/ControlChannel.h
    source/protocol/FileChannel.cpp
    source/protocol/FileChannel.h
    source/protocol/OutboundConnector.cpp
    source/protocol/OutboundConnector.h
    source/signals.cpp
    source/signals.hpp
    source/tor.cpp
    source/tor.hpp
    source/tor/AddOnionCommand.cpp
    source/tor/AddOnionCommand.h
    source/tor/AuthenticateCommand.cpp
    source/tor/AuthenticateCommand.h
    source/tor/GetConfCommand.cpp
    source/tor/GetConfCommand.h
    source/tor/HiddenService.cpp
    source/tor/HiddenService.h
    source/tor/ProtocolInfoCommand.cpp
    source/tor/ProtocolInfoCommand.h
    source/tor/SetConfCommand.cpp
    source/tor/SetConfCommand.h
    source/tor/TorControl.cpp
    source/tor/TorControl.h
    source/tor/TorControlCommand.cpp
    source/tor/TorControlCommand.h
    source/tor/TorControlSocket.cpp
    source/tor/TorControlSocket.h
    source/tor/TorManager.cpp
    source/tor/TorManager.h
    source/tor/TorProcess.cpp
    source/tor/TorProcess.h
    source/tor/TorProcess_p.h
    source/tor/TorSocket.cpp
    source/tor/TorSocket.h
    source/tor_stubs.cpp
    source/user.cpp
    source/user.hpp
    source/utils/CryptoKey.cpp
    source/utils/CryptoKey.h
    source/utils/PendingOperation.cpp
    source/utils/PendingOperation.h
    source/utils/SecureRNG.cpp
    source/utils/SecureRNG.h
    source/utils/StringUtil.cpp
    source/utils/StringUtil.h
    source/utils/Useful.h)
target_precompile_headers(tego PRIVATE source/precomp.h)

include(lto)
include(compiler_opts)
# enables compiler specific warnings/sanitizers if requested
setup_compiler(tego)

target_compile_features(tego PRIVATE cxx_std_20)

target_include_directories(tego PRIVATE source/)
target_include_directories(tego PUBLIC include/)

set(EXTERN_DIR "../extern/")

# protobuf
target_include_directories(tego PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/protocol)
target_link_libraries(tego PRIVATE ${Protobuf_LIBRARIES})
protobuf_generate_cpp(
    PROTO_SRC
    PROTO_HDR
    source/protocol/FileChannel.proto
    source/protocol/ContactRequestChannel.proto
    source/protocol/ControlChannel.proto
    source/protocol/ChatChannel.proto
    source/protocol/AuthHiddenService.proto)
target_sources(tego PRIVATE ${PROTO_SRC} ${PROTO_HDR})

# tor
target_include_directories(tego PRIVATE ${EXTERN_DIR}/tor ${EXTERN_DIR}/tor/src ${EXTERN_DIR}/tor/src/ext)

list(
    APPEND
    TOR_SRC
    ${EXTERN_DIR}/tor/src/ext/ed25519/donna/ed25519_tor.c
    ${EXTERN_DIR}/tor/src/lib/encoding/binascii.c
    ${EXTERN_DIR}/tor/src/lib/crypt_ops/crypto_digest_openssl.c)
target_sources(tego PRIVATE ${TOR_SRC})

# silence warnings on the tor sources
if (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set_source_files_properties(${TOR_SRC} PROPERTIES COMPILE_FLAGS "-w")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_source_files_properties(${TOR_SRC} PROPERTIES COMPILE_FLAGS "/w")
endif ()

target_link_libraries(tego PRIVATE fmt::fmt)
target_link_libraries(tego PRIVATE OpenSSL::Crypto)

# QT
target_link_libraries(
    tego
    PRIVATE Qt${QT_VERSION_MAJOR}::Core
            Qt${QT_VERSION_MAJOR}::Widgets
            Qt${QT_VERSION_MAJOR}::Network
            Qt${QT_VERSION_MAJOR}::Qml
            Qt${QT_VERSION_MAJOR}::Quick)

if (APPLE)
    target_link_libraries(tego PRIVATE Qt${QT_VERSION_MAJOR}::MacExtras)
endif ()

target_link_libraries(tego PRIVATE Threads::Threads)

if ("${CMAKE_BUILD_TYPE}" MATCHES "Rel.*" OR "${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    target_compile_definitions(tego PRIVATE QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT)
endif ()

add_subdirectory(test)
