QMAKE_INCLUDES = $${PWD}/../qmake_includes

include($${QMAKE_INCLUDES}/artifacts.pri)
include($${QMAKE_INCLUDES}/compiler_flags.pri)

TEMPLATE = lib
TARGET = tego
CONFIG += staticlib

QT += core gui network quick widgets
macx {
    QT += macextras
}

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
    source/ed25519.hpo\
    source/context.hpp\
    source/signals.hpp

SOURCES +=\
    source/libtego.cpp\
    source/delete.cpp\
    source/error.cpp\
    source/tor_stubs.cpp\
    source/ed25519.cpp\
    source/logger.cpp\
    source/context.cpp\
    source/signals.cpp


# external
INCLUDEPATH +=\
    include\
    source\
    $${TOR_INCLUDE_DIR}

#
# from libtego_ui
#

SOURCES += \
    source/core/ContactIDValidator.cpp \
    source/core/ContactsManager.cpp \
    source/core/ContactUser.cpp \
    source/core/ConversationModel.cpp \
    source/core/IdentityManager.cpp \
    source/core/IncomingRequestManager.cpp \
    source/core/OutgoingContactRequest.cpp \
    source/core/UserIdentity.cpp \
    source/tor/AddOnionCommand.cpp \
    source/tor/AuthenticateCommand.cpp \
    source/tor/GetConfCommand.cpp \
    source/tor/HiddenService.cpp \
    source/tor/ProtocolInfoCommand.cpp \
    source/tor/SetConfCommand.cpp \
    source/tor/TorControl.cpp \
    source/tor/TorControlCommand.cpp \
    source/tor/TorControlSocket.cpp \
    source/tor/TorManager.cpp \
    source/tor/TorProcess.cpp \
    source/tor/TorSocket.cpp \
    source/utils/CryptoKey.cpp \
    source/utils/PendingOperation.cpp \
    source/utils/SecureRNG.cpp \
    source/utils/Settings.cpp \
    source/utils/StringUtil.cpp


HEADERS += \
    source/core/ContactIDValidator.h \
    source/core/ContactsManager.h \
    source/core/ContactUser.h \
    source/core/ConversationModel.h \
    source/core/IdentityManager.h \
    source/core/IncomingRequestManager.h \
    source/core/OutgoingContactRequest.h \
    source/core/UserIdentity.h \
    source/tor/AddOnionCommand.h \
    source/tor/AuthenticateCommand.h \
    source/tor/GetConfCommand.h \
    source/tor/HiddenService.h \
    source/tor/ProtocolInfoCommand.h \
    source/tor/SetConfCommand.h \
    source/tor/TorControl.h \
    source/tor/TorControlCommand.h \
    source/tor/TorControlSocket.h \
    source/tor/TorManager.h \
    source/tor/TorProcess.h \
    source/tor/TorProcess_p.h \
    source/tor/TorSocket.h \
    source/utils/CryptoKey.h \
    source/utils/PendingOperation.h \
    source/utils/SecureRNG.h \
    source/utils/Settings.h \
    source/utils/StringUtil.h

SOURCES += \
    source/protocol/AuthHiddenServiceChannel.cpp \
    source/protocol/Channel.cpp \
    source/protocol/ChatChannel.cpp \
    source/protocol/Connection.cpp \
    source/protocol/ContactRequestChannel.cpp \
    source/protocol/ControlChannel.cpp \
    source/protocol/OutboundConnector.cpp

HEADERS += \
    source/protocol/AuthHiddenServiceChannel.h \
    source/protocol/Channel.h \
    source/protocol/Channel_p.h \
    source/protocol/ChatChannel.h \
    source/protocol/Connection.h \
    source/protocol/Connection_p.h \
    source/protocol/ContactRequestChannel.h \
    source/protocol/ControlChannel.h \
    source/protocol/OutboundConnector.h

include($${QMAKE_INCLUDES}/protobuf.pri)

PROTOS += \
    source/protocol/AuthHiddenService.proto \
    source/protocol/ChatChannel.proto \
    source/protocol/ContactRequestChannel.proto \
    source/protocol/ControlChannel.proto

include($${QMAKE_INCLUDES}/openssl.pri)