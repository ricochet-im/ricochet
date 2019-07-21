QT += core gui network quick widgets

CONFIG += c++11

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
    $${SRC}/tor/TorControl.cpp \
    $${SRC}/tor/TorControlSocket.cpp \
    $${SRC}/tor/TorControlCommand.cpp \
    $${SRC}/tor/ProtocolInfoCommand.cpp \
    $${SRC}/tor/AuthenticateCommand.cpp \
    $${SRC}/tor/SetConfCommand.cpp \
    $${SRC}/tor/AddOnionCommand.cpp \
    $${SRC}/tor/GetConfCommand.cpp \
    $${SRC}/tor/HiddenService.cpp \
    $${SRC}/tor/TorProcess.cpp \
    $${SRC}/tor/TorManager.cpp \
    $${SRC}/tor/TorSocket.cpp \
    $${SRC}/core/ContactIDValidator.cpp \
    $${SRC}/core/ContactsManager.cpp \
    $${SRC}/core/ContactUser.cpp \
    $${SRC}/core/OutgoingContactRequest.cpp \
    $${SRC}/core/IncomingRequestManager.cpp \
    $${SRC}/core/UserIdentity.cpp \
    $${SRC}/core/IdentityManager.cpp \
    $${SRC}/core/ConversationModel.cpp \
    $${SRC}/utils/StringUtil.cpp \
    $${SRC}/utils/CryptoKey.cpp \
    $${SRC}/utils/SecureRNG.cpp \
    $${SRC}/utils/Settings.cpp \
    $${SRC}/utils/PendingOperation.cpp \
    $${SRC}/ui/ContactsModel.cpp \
    $${SRC}/ui/MainWindow.cpp \
    $${SRC}/ui/LinkedText.cpp \
    $${SRC}/ui/LanguagesModel.cpp

HEADERS += $${SRC}/ui/MainWindow.h \
    $${SRC}/ui/ContactsModel.h \
    $${SRC}/tor/TorControl.h \
    $${SRC}/tor/TorControlSocket.h \
    $${SRC}/tor/TorControlCommand.h \
    $${SRC}/tor/ProtocolInfoCommand.h \
    $${SRC}/tor/AuthenticateCommand.h \
    $${SRC}/tor/SetConfCommand.h \
    $${SRC}/tor/AddOnionCommand.h \
    $${SRC}/utils/StringUtil.h \
    $${SRC}/core/ContactsManager.h \
    $${SRC}/core/ContactUser.h \
    $${SRC}/tor/GetConfCommand.h \
    $${SRC}/tor/HiddenService.h \
    $${SRC}/utils/CryptoKey.h \
    $${SRC}/utils/SecureRNG.h \
    $${SRC}/core/OutgoingContactRequest.h \
    $${SRC}/core/IncomingRequestManager.h \
    $${SRC}/core/ContactIDValidator.h \
    $${SRC}/core/UserIdentity.h \
    $${SRC}/core/IdentityManager.h \
    $${SRC}/core/ConversationModel.h \
    $${SRC}/tor/TorProcess.h \
    $${SRC}/tor/TorProcess_p.h \
    $${SRC}/tor/TorManager.h \
    $${SRC}/tor/TorSocket.h \
    $${SRC}/utils/Settings.h \
    $${SRC}/utils/PendingOperation.h \
    $${SRC}/ui/LinkedText.h \
    $${SRC}/ui/LanguagesModel.h

SOURCES += $${SRC}/protocol/Channel.cpp \
    $${SRC}/protocol/ControlChannel.cpp \
    $${SRC}/protocol/Connection.cpp \
    $${SRC}/protocol/OutboundConnector.cpp \
    $${SRC}/protocol/AuthHiddenServiceChannel.cpp \
    $${SRC}/protocol/ChatChannel.cpp \
    $${SRC}/protocol/ContactRequestChannel.cpp

HEADERS += $${SRC}/protocol/Channel.h \
    $${SRC}/protocol/Channel_p.h \
    $${SRC}/protocol/ControlChannel.h \
    $${SRC}/protocol/Connection.h \
    $${SRC}/protocol/Connection_p.h \
    $${SRC}/protocol/OutboundConnector.h \
    $${SRC}/protocol/AuthHiddenServiceChannel.h \
    $${SRC}/protocol/ChatChannel.h \
    $${SRC}/protocol/ContactRequestChannel.h

PROTOS += $${SRC}/protocol/ControlChannel.proto \
    $${SRC}/protocol/AuthHiddenService.proto \
    $${SRC}/protocol/ChatChannel.proto \
    $${SRC}/protocol/ContactRequestChannel.proto
