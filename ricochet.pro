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
QT += core gui network quick widgets
CONFIG += c++11

VERSION = 1.1.4

# Use CONFIG+=no-hardened to disable compiler hardening options
!CONFIG(no-hardened) {
    CONFIG += hardened
    include(hardened.pri)
}

# Pass DEFINES+=RICOCHET_NO_PORTABLE for a system-wide installation

CONFIG(release,debug|release):DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT

contains(DEFINES, RICOCHET_NO_PORTABLE) {
    unix:!macx {
        target.path = $$PREFIX/bin
        shortcut.path = $$PREFIX/share/applications
        shortcut.files = src/ricochet.desktop
        icon.path = $$PREFIX/share/icons/hicolor/48x48/apps/
        icon.files = icons/ricochet.png
        scalable_icon.path = $$PREFIX/share/icons/hicolor/scalable/apps/
        scalable_icon.files = icons/ricochet.svg
        INSTALLS += target shortcut icon scalable_icon
        QMAKE_CLEAN += contrib/usr.bin.ricochet
        contains(DEFINES, APPARMOR) {
            apparmor_profile.extra = cp -f $${_PRO_FILE_PWD_}/contrib/usr.bin.ricochet-apparmor $${_PRO_FILE_PWD_}/contrib/usr.bin.ricochet
            apparmor_profile.files = contrib/usr.bin.ricochet
            QMAKE_CLEAN += contrib/usr.bin.ricochet
            !isEmpty(APPARMORDIR) {
                    apparmor_profile.path = $${APPARMORDIR}/
            } else {
                    apparmor_profile.path = /etc/apparmor.d/
            }
            INSTALLS += apparmor_profile
        }

        exists(tor) {
            message(Adding bundled Tor to installations)
            bundletor.path = $$PREFIX/lib/ricochet/tor/
            bundletor.files = tor/*
            INSTALLS += bundletor
            DEFINES += BUNDLED_TOR_PATH=\\\"$$PREFIX/lib/ricochet/tor/\\\"
        }
    }
}

macx {
    CONFIG += bundle force_debug_plist
    QT += macextras

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
        !equals(OPENSSLDIR, "/usr") {
            # adding /usr/include to INCLUDEPATH breaks STL's include logic
            INCLUDEPATH += $${OPENSSLDIR}/include
        }
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

# Exclude unneeded plugins from static builds
QTPLUGIN.playlistformats = -
QTPLUGIN.imageformats = -
QTPLUGIN.printsupport = -
QTPLUGIN.mediaservice = -
# Include Linux input plugins, which are missing by default, to provide complex input support. See issue #60.
unix:!macx:QTPLUGIN.platforminputcontexts = composeplatforminputcontextplugin ibusplatforminputcontextplugin

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
    src/tor/AddOnionCommand.cpp \
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
    src/ui/LanguagesModel.cpp

HEADERS += src/ui/MainWindow.h \
    src/ui/ContactsModel.h \
    src/tor/TorControl.h \
    src/tor/TorControlSocket.h \
    src/tor/TorControlCommand.h \
    src/tor/ProtocolInfoCommand.h \
    src/tor/AuthenticateCommand.h \
    src/tor/SetConfCommand.h \
    src/tor/AddOnionCommand.h \
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
    src/ui/LanguagesModel.h

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
    ricochet_en \
    ricochet_it \
    ricochet_es \
    ricochet_da \
    ricochet_pl \
    ricochet_pt_BR \
    ricochet_de \
    ricochet_bg \
    ricochet_cs \
    ricochet_fi \
    ricochet_fr \
    ricochet_ru \
    ricochet_uk \
    ricochet_tr \
    ricochet_nl_NL \
    ricochet_fil_PH \
    ricochet_sv \
    ricochet_he \
    ricochet_sl \
    ricochet_zh \
    ricochet_et_EE \
    ricochet_it_IT \
    ricochet_nb \
    ricochet_pt_PT \
    ricochet_sq \
    ricochet_zh_HK \
    ricochet_ja

# Only build translations when creating the primary makefile.
!build_pass: {
    contains(QMAKE_HOST.os,Windows):QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    for (translation, TRANSLATIONS) {
        system($$QMAKE_LRELEASE translation/$${translation}.ts -qm translation/$${translation}.qm)
    }
}

RESOURCES += translation/embedded.qrc
