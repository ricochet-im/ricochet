# Ricochet - https://ricochet.im/
# Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#    * Redistributions in binary form must reproduce the above
#      copyright notice, this list of conditions and the following disclaimer
#      in the documentation and/or other materials provided with the
#      distribution.
#
#    * Neither the names of the copyright owners nor the names of its
#      contributors may be used to endorse or promote products derived from
#      this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

lessThan(QT_MAJOR_VERSION,5)|lessThan(QT_MINOR_VERSION,1) {
    error("Qt 5.1 or greater is required. You can build your own, or get the SDK at https://qt-project.org/downloads")
}

TARGET = ricochet
TEMPLATE = app
QT += core gui network quick widgets multimedia
CONFIG += c++11

VERSION = 1.1.0

# Pass DEFINES+=RICOCHET_NO_PORTABLE for a system-wide installation

CONFIG(release,debug|release):DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT

contains(DEFINES, RICOCHET_NO_PORTABLE) {
    unix:!macx {
        target.path = /usr/bin
        shortcut.path = /usr/share/applications
        shortcut.files = src/ricochet.desktop
        icon.path = /usr/share/icons/hicolor/48x48/apps/
        icon.files = icons/ricochet.png
        scalable_icon.path = /usr/share/icons/hicolor/scalable/apps/
        scalable_icon.files = icons/ricochet.svg
        INSTALLS += target shortcut icon scalable_icon

        exists(tor) {
            message(Adding bundled Tor to installations)
            bundletor.path = /usr/lib/ricochet/tor/
            bundletor.files = tor/*
            INSTALLS += bundletor
            DEFINES += BUNDLED_TOR_PATH=\\\"/usr/lib/ricochet/tor/\\\"
        }
    }
}

macx {
    CONFIG += bundle force_debug_plist

    # Qt 5.4 introduces a bug that breaks QMAKE_INFO_PLIST when qmake has a relative path.
    # Work around by copying Info.plist directly.
    greaterThan(QT_MAJOR_VERSION,5)|greaterThan(QT_MINOR_VERSION,4) {
        QMAKE_INFO_PLIST = src/Info.plist
    } else:equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,4) {
        QMAKE_INFO_PLIST = src/Info.plist
    } else {
        CONFIG += no_plist
        QMAKE_POST_LINK += cp $${_PRO_FILE_PWD_}/src/Info.plist $${OUT_PWD}/$${TARGET}.app/Contents/;
    }

    exists(tor) {
        # Copy the entire tor/ directory, which should contain tor/tor (the binary itself)
        QMAKE_POST_LINK += cp -R $${_PRO_FILE_PWD_}/tor $${OUT_PWD}/$${TARGET}.app/Contents/MacOS/;
    }

    icons.files = icons/Ricochet.icns
    icons.path = Contents/Resources/
    QMAKE_BUNDLE_DATA += icons
}

CONFIG += debug_and_release

# Create a pdb for release builds as well, to enable debugging
win32-msvc2008|win32-msvc2010 {
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF,ICF
}

INCLUDEPATH += src

unix:!macx {
    !isEmpty(OPENSSLDIR) {
        INCLUDEPATH += $${OPENSSLDIR}/include
        LIBS += -L$${OPENSSLDIR}/lib -lcrypto
    } else {
        CONFIG += link_pkgconfig
        PKGCONFIG += libcrypto
    }
}
win32 {
    isEmpty(OPENSSLDIR):error(You must pass OPENSSLDIR=path/to/openssl to qmake on this platform)

    INCLUDEPATH += $${OPENSSLDIR}/include

    win32-g++ {
        LIBS += -L$${OPENSSLDIR}/lib -lcrypto
    } else {
        LIBS += -L$${OPENSSLDIR}/lib -llibeay32
    }

    # required by openssl
    LIBS += -lUser32 -lGdi32 -ladvapi32
}
macx:LIBS += -lcrypto

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

SOURCES += src/main.cpp \
    src/ui/MainWindow.cpp \
    src/ui/ContactsModel.cpp \
    src/tor/TorControl.cpp \
    src/tor/TorControlSocket.cpp \
    src/tor/TorControlCommand.cpp \
    src/tor/ProtocolInfoCommand.cpp \
    src/tor/AuthenticateCommand.cpp \
    src/tor/SetConfCommand.cpp \
    src/utils/StringUtil.cpp \
    src/core/ContactsManager.cpp \
    src/core/ContactUser.cpp \
    src/tor/GetConfCommand.cpp \
    src/tor/HiddenService.cpp \
    src/utils/CryptoKey.cpp \
    src/utils/SecureRNG.cpp \
    src/core/OutgoingContactRequest.cpp \
    src/core/IncomingRequestManager.cpp \
    src/core/ContactIDValidator.cpp \
    src/core/UserIdentity.cpp \
    src/core/IdentityManager.cpp \
    src/core/ConversationModel.cpp \
    src/tor/TorProcess.cpp \
    src/tor/TorManager.cpp \
    src/tor/TorSocket.cpp \
    src/ui/LinkedText.cpp \
    src/utils/Settings.cpp \
    src/utils/PendingOperation.cpp \
    src/ui/AudioNotification.cpp \
    src/utils/AudioPlayer.cpp

HEADERS += src/ui/MainWindow.h \
    src/ui/ContactsModel.h \
    src/tor/TorControl.h \
    src/tor/TorControlSocket.h \
    src/tor/TorControlCommand.h \
    src/tor/ProtocolInfoCommand.h \
    src/tor/AuthenticateCommand.h \
    src/tor/SetConfCommand.h \
    src/utils/StringUtil.h \
    src/core/ContactsManager.h \
    src/core/ContactUser.h \
    src/tor/GetConfCommand.h \
    src/tor/HiddenService.h \
    src/utils/CryptoKey.h \
    src/utils/SecureRNG.h \
    src/core/OutgoingContactRequest.h \
    src/core/IncomingRequestManager.h \
    src/core/ContactIDValidator.h \
    src/core/UserIdentity.h \
    src/core/IdentityManager.h \
    src/core/ConversationModel.h \
    src/tor/TorProcess.h \
    src/tor/TorProcess_p.h \
    src/tor/TorManager.h \
    src/tor/TorSocket.h \
    src/ui/LinkedText.h \
    src/utils/Settings.h \
    src/utils/PendingOperation.h \
    src/ui/AudioNotification.h \
    src/utils/AudioPlayer.h

SOURCES += src/protocol/Channel.cpp \
    src/protocol/ControlChannel.cpp \
    src/protocol/Connection.cpp \
    src/protocol/OutboundConnector.cpp \
    src/protocol/AuthHiddenServiceChannel.cpp \
    src/protocol/ChatChannel.cpp \
    src/protocol/ContactRequestChannel.cpp

HEADERS += src/protocol/Channel.h \
    src/protocol/Channel_p.h \
    src/protocol/ControlChannel.h \
    src/protocol/Connection.h \
    src/protocol/Connection_p.h \
    src/protocol/OutboundConnector.h \
    src/protocol/AuthHiddenServiceChannel.h \
    src/protocol/ChatChannel.h \
    src/protocol/ContactRequestChannel.h

include(protobuf.pri)
PROTOS += src/protocol/ControlChannel.proto \
    src/protocol/AuthHiddenService.proto \
    src/protocol/ChatChannel.proto \
    src/protocol/ContactRequestChannel.proto

# QML
RESOURCES += src/ui/qml/qml.qrc \
    icons/icons.qrc \
    sounds/sounds.qrc

win32:RC_ICONS = icons/ricochet.ico
OTHER_FILES += src/ui/qml/*
lupdate_only {
    SOURCES += src/ui/qml/*.qml
}

# Translations
TRANSLATIONS += \
    translation/ricochet_en.ts \
    translation/ricochet_it.ts \
    translation/ricochet_es.ts \
    translation/ricochet_da.ts \
    translation/ricochet_pt_BR.ts \
    translation/ricochet_de.ts \
    translation/ricochet_bg.ts \
    translation/ricochet_cs.ts \
    translation/ricochet_fi.ts \
    translation/ricochet_fr.ts \
    translation/ricochet_ru.ts \
    translation/ricochet_uk.ts \
    translation/ricochet_tr.ts \
    translation/ricochet_nl_NL.ts \
    translation/ricochet_fil_PH.ts \
    translation/ricochet_sv.ts

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}

updateqm.input = TRANSLATIONS
updateqm.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.commands = $$QMAKE_LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.qm
updateqm.CONFIG += no_link target_predeps
QMAKE_EXTRA_COMPILERS += updateqm

RESOURCES += translation/embedded.qrc
