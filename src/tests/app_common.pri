QT += core gui network quick widgets

macx {
    QT += macextras
}

win32|mac {
    # For mac, this is necessary because homebrew does not link openssl .pc to
    # /usr/local/lib/pkgconfig (presumably because it used to be a system
    # package).
    #
    # Unfortunately, it is no longer really a system package, and we really
    # need to know where it is.
    isEmpty(OPENSSLDIR): error(You must pass OPENSSLDIR=path/to/openssl to qmake on this platform)
}

unix {
    # get us onto the latest c++
    QMAKE_CXXFLAGS += --std=c++2a

    !isEmpty(OPENSSLDIR) {
        INCLUDEPATH += $${OPENSSLDIR}/include
        LIBS += -L$${OPENSSLDIR}/lib -lcrypto
    } else {
        CONFIG += link_pkgconfig
        PKGCONFIG += libcrypto
    }
}
win32 {
    INCLUDEPATH += $${OPENSSLDIR}/include

    win32-g++ {
        LIBS += -L$${OPENSSLDIR}/lib -lcrypto
    } else {
        LIBS += -L$${OPENSSLDIR}/lib -llibeay32
    }

    # required by openssl
    LIBS += -luser32 -lgdi32 -ladvapi32
}

SOURCES += \
    $${SRC}/tego-ui/tor/TorControl.cpp \
    $${SRC}/tego-ui/tor/TorControlSocket.cpp \
    $${SRC}/tego-ui/tor/TorControlCommand.cpp \
    $${SRC}/tego-ui/tor/ProtocolInfoCommand.cpp \
    $${SRC}/tego-ui/tor/AuthenticateCommand.cpp \
    $${SRC}/tego-ui/tor/SetConfCommand.cpp \
    $${SRC}/tego-ui/tor/AddOnionCommand.cpp \
    $${SRC}/tego-ui/tor/GetConfCommand.cpp \
    $${SRC}/tego-ui/tor/HiddenService.cpp \
    $${SRC}/tego-ui/tor/TorProcess.cpp \
    $${SRC}/tego-ui/tor/TorManager.cpp \
    $${SRC}/tego-ui/tor/TorSocket.cpp \
    $${SRC}/tego-ui/core/ContactIDValidator.cpp \
    $${SRC}/tego-ui/core/ContactsManager.cpp \
    $${SRC}/tego-ui/core/ContactUser.cpp \
    $${SRC}/tego-ui/core/OutgoingContactRequest.cpp \
    $${SRC}/tego-ui/core/IncomingRequestManager.cpp \
    $${SRC}/tego-ui/core/UserIdentity.cpp \
    $${SRC}/tego-ui/core/IdentityManager.cpp \
    $${SRC}/tego-ui/core/ConversationModel.cpp \
    $${SRC}/tego-ui/utils/StringUtil.cpp \
    $${SRC}/tego-ui/utils/CryptoKey.cpp \
    $${SRC}/tego-ui/utils/SecureRNG.cpp \
    $${SRC}/tego-ui/utils/Settings.cpp \
    $${SRC}/tego-ui/utils/PendingOperation.cpp \
    $${SRC}/tego-ui/ui/ContactsModel.cpp \
    $${SRC}/tego-ui/ui/MainWindow.cpp \
    $${SRC}/tego-ui/ui/LinkedText.cpp \
    $${SRC}/tego-ui/ui/LanguagesModel.cpp

HEADERS += $${SRC}/tego-ui/ui/MainWindow.h \
    $${SRC}/tego-ui/ui/ContactsModel.h \
    $${SRC}/tego-ui/tor/TorControl.h \
    $${SRC}/tego-ui/tor/TorControlSocket.h \
    $${SRC}/tego-ui/tor/TorControlCommand.h \
    $${SRC}/tego-ui/tor/ProtocolInfoCommand.h \
    $${SRC}/tego-ui/tor/AuthenticateCommand.h \
    $${SRC}/tego-ui/tor/SetConfCommand.h \
    $${SRC}/tego-ui/tor/AddOnionCommand.h \
    $${SRC}/tego-ui/utils/StringUtil.h \
    $${SRC}/tego-ui/core/ContactsManager.h \
    $${SRC}/tego-ui/core/ContactUser.h \
    $${SRC}/tego-ui/tor/GetConfCommand.h \
    $${SRC}/tego-ui/tor/HiddenService.h \
    $${SRC}/tego-ui/utils/CryptoKey.h \
    $${SRC}/tego-ui/utils/SecureRNG.h \
    $${SRC}/tego-ui/core/OutgoingContactRequest.h \
    $${SRC}/tego-ui/core/IncomingRequestManager.h \
    $${SRC}/tego-ui/core/ContactIDValidator.h \
    $${SRC}/tego-ui/core/UserIdentity.h \
    $${SRC}/tego-ui/core/IdentityManager.h \
    $${SRC}/tego-ui/core/ConversationModel.h \
    $${SRC}/tego-ui/tor/TorProcess.h \
    $${SRC}/tego-ui/tor/TorProcess_p.h \
    $${SRC}/tego-ui/tor/TorManager.h \
    $${SRC}/tego-ui/tor/TorSocket.h \
    $${SRC}/tego-ui/utils/Settings.h \
    $${SRC}/tego-ui/utils/PendingOperation.h \
    $${SRC}/tego-ui/ui/LinkedText.h \
    $${SRC}/tego-ui/ui/LanguagesModel.h

SOURCES += $${SRC}/tego-ui/protocol/Channel.cpp \
    $${SRC}/tego-ui/protocol/ControlChannel.cpp \
    $${SRC}/tego-ui/protocol/Connection.cpp \
    $${SRC}/tego-ui/protocol/OutboundConnector.cpp \
    $${SRC}/tego-ui/protocol/AuthHiddenServiceChannel.cpp \
    $${SRC}/tego-ui/protocol/ChatChannel.cpp \
    $${SRC}/tego-ui/protocol/ContactRequestChannel.cpp

HEADERS += $${SRC}/tego-ui/protocol/Channel.h \
    $${SRC}/tego-ui/protocol/Channel_p.h \
    $${SRC}/tego-ui/protocol/ControlChannel.h \
    $${SRC}/tego-ui/protocol/Connection.h \
    $${SRC}/tego-ui/protocol/Connection_p.h \
    $${SRC}/tego-ui/protocol/OutboundConnector.h \
    $${SRC}/tego-ui/protocol/AuthHiddenServiceChannel.h \
    $${SRC}/tego-ui/protocol/ChatChannel.h \
    $${SRC}/tego-ui/protocol/ContactRequestChannel.h

# fmt lib
SOURCE +=\
    $${SRC}/tego-ui/fmt/format.cc \
    $${SRC}/tego-ui/fmt/posix.cc

HEADERS +=\
    $${SRC}/tego-ui/fmt/core.h \
    $${SRC}/tego-ui/fmt/format.h \
    $${SRC}/tego-ui/fmt/format-inl.h \
    $${SRC}/tego-ui/fmt/ostream.h \
    $${SRC}/tego-ui/fmt/posix.h \
    $${SRC}/tego-ui/fmt/printf.h \
    $${SRC}/tego-ui/fmt/ranges.h \
    $${SRC}/tego-ui/fmt/time.h

# custom
HEADERS +=\
    $${SRC}/tego-ui/logger.hpp

SOURCES +=\
    $${SRC}/tego-ui/logger.cpp

PROTOS += $${SRC}/tego-ui/protocol/ControlChannel.proto \
    $${SRC}/tego-ui/protocol/AuthHiddenService.proto \
    $${SRC}/tego-ui/protocol/ChatChannel.proto \
    $${SRC}/tego-ui/protocol/ContactRequestChannel.proto

