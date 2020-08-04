# Ricochet Refresh - https://blueprint-freespeech.github.io/refresh-site/
# Copyright (C) 2019, Blueprint for Free Speech  <ricochet@blueprintforfreespeech.net>
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

lessThan(QT_MAJOR_VERSION,5)|lessThan(QT_MINOR_VERSION,15) {
    error("Qt 5.15 or greater is required. You can build your own, or get the SDK at https://qt-project.org/downloads")
}

include($${PWD}/../qmake_includes/artifacts.pri)

TARGET = ricochet-refresh
TEMPLATE = app
QT += core gui network quick widgets

VERSION = 1.1.4

# Use CONFIG+=no-hardened to disable compiler hardening options
!CONFIG(no-hardened) {
    CONFIG += hardened
    include($${PWD}/../qmake_includes/hardened.pri)
}

# Pass DEFINES+=RICOCHET_NO_PORTABLE for a system-wide installation

CONFIG(release,debug|release):DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT

contains(DEFINES, RICOCHET_NO_PORTABLE) {
    unix:!macx {
        target.path = /usr/bin
        shortcut.path = /usr/share/applications
        shortcut.files = ricochet.desktop
        icon.path = /usr/share/icons/hicolor/48x48/apps/
        icon.files = icons/ricochet_refresh.png
        scalable_icon.path = /usr/share/icons/hicolor/scalable/apps/
        scalable_icon.files = icons/ricochet_refresh.svg
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
            bundletor.path = /usr/lib/ricochet/tor/
            bundletor.files = tor/*
            INSTALLS += bundletor
            DEFINES += BUNDLED_TOR_PATH=\\\"/usr/lib/ricochet/tor/\\\"
        }
    }
}

macx {
    CONFIG += bundle force_debug_plist
    QT += macextras

    # Qt 5.4 introduces a bug that breaks QMAKE_INFO_PLIST when qmake has a relative path.
    # Work around by copying Info.plist directly.
    greaterThan(QT_MAJOR_VERSION,5)|greaterThan(QT_MINOR_VERSION,4) {
        QMAKE_INFO_PLIST = Info.plist
    } else:equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,4) {
        QMAKE_INFO_PLIST = Info.plist
    } else {
        CONFIG += no_plist
        QMAKE_POST_LINK += cp $${_PRO_FILE_PWD_}/Info.plist $${OUT_PWD}/$${TARGET}.app/Contents/;
    }

    exists(tor) {
        # Copy the entire tor/ directory, which should contain tor/tor (the binary itself)
        QMAKE_POST_LINK += cp -R $${_PRO_FILE_PWD_}/tor $${OUT_PWD}/$${TARGET}.app/Contents/MacOS/;
    }

    icons.files = icons/ricochet_refresh.icns
    icons.path = Contents/Resources/
    QMAKE_BUNDLE_DATA += icons
}

# Create a pdb for release builds as well, to enable debugging
win32-msvc2008|win32-msvc2010 {
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF,ICF
}

INCLUDEPATH += $${PWD}

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

include($${PWD}/../libtego/libtego.pri)

# Exclude unneeded plugins from static builds
QTPLUGIN.playlistformats = -
QTPLUGIN.imageformats = -
QTPLUGIN.printsupport = -
QTPLUGIN.mediaservice = -
# Include Linux input plugins, which are missing by default, to provide complex input support. See issue #60.
unix:!macx:QTPLUGIN.platforminputcontexts = composeplatforminputcontextplugin ibusplatforminputcontextplugin

DEFINES += QT_NO_CAST_TO_ASCII

SOURCES += main.cpp \
    ui/MainWindow.cpp \
    ui/ContactsModel.cpp \
    tor/TorControl.cpp \
    tor/TorControlSocket.cpp \
    tor/TorControlCommand.cpp \
    tor/ProtocolInfoCommand.cpp \
    tor/AuthenticateCommand.cpp \
    tor/SetConfCommand.cpp \
    tor/AddOnionCommand.cpp \
    utils/StringUtil.cpp \
    core/ContactsManager.cpp \
    core/ContactUser.cpp \
    tor/GetConfCommand.cpp \
    tor/HiddenService.cpp \
    utils/CryptoKey.cpp \
    utils/SecureRNG.cpp \
    core/OutgoingContactRequest.cpp \
    core/IncomingRequestManager.cpp \
    core/ContactIDValidator.cpp \
    core/UserIdentity.cpp \
    core/IdentityManager.cpp \
    core/ConversationModel.cpp \
    tor/TorProcess.cpp \
    tor/TorManager.cpp \
    tor/TorSocket.cpp \
    ui/LinkedText.cpp \
    utils/Settings.cpp \
    utils/PendingOperation.cpp \
    ui/LanguagesModel.cpp

HEADERS += ui/MainWindow.h \
    ui/ContactsModel.h \
    tor/TorControl.h \
    tor/TorControlSocket.h \
    tor/TorControlCommand.h \
    tor/ProtocolInfoCommand.h \
    tor/AuthenticateCommand.h \
    tor/SetConfCommand.h \
    tor/AddOnionCommand.h \
    utils/StringUtil.h \
    core/ContactsManager.h \
    core/ContactUser.h \
    tor/GetConfCommand.h \
    tor/HiddenService.h \
    utils/CryptoKey.h \
    utils/SecureRNG.h \
    core/OutgoingContactRequest.h \
    core/IncomingRequestManager.h \
    core/ContactIDValidator.h \
    core/UserIdentity.h \
    core/IdentityManager.h \
    core/ConversationModel.h \
    tor/TorProcess.h \
    tor/TorProcess_p.h \
    tor/TorManager.h \
    tor/TorSocket.h \
    ui/LinkedText.h \
    utils/Settings.h \
    utils/PendingOperation.h \
    ui/LanguagesModel.h

SOURCES += protocol/Channel.cpp \
    protocol/ControlChannel.cpp \
    protocol/Connection.cpp \
    protocol/OutboundConnector.cpp \
    protocol/AuthHiddenServiceChannel.cpp \
    protocol/ChatChannel.cpp \
    protocol/ContactRequestChannel.cpp

HEADERS += protocol/Channel.h \
    protocol/Channel_p.h \
    protocol/ControlChannel.h \
    protocol/Connection.h \
    protocol/Connection_p.h \
    protocol/OutboundConnector.h \
    protocol/AuthHiddenServiceChannel.h \
    protocol/ChatChannel.h \
    protocol/ContactRequestChannel.h

# fmt lib
SOURCE +=\
    fmt/format.cc \
    fmt/posix.cc

HEADERS +=\
    fmt/core.h \
    fmt/format.h \
    fmt/format-inl.h \
    fmt/ostream.h \
    fmt/posix.h \
    fmt/printf.h \
    fmt/ranges.h \
    fmt/time.h

# custom
HEADERS +=\
    logger.hpp

SOURCES +=\
    logger.cpp

include($${PWD}/../qmake_includes/protobuf.pri)

PROTOS += protocol/ControlChannel.proto \
    protocol/AuthHiddenService.proto \
    protocol/ChatChannel.proto \
    protocol/ContactRequestChannel.proto

# QML
RESOURCES += ui/qml/qml.qrc \
    icons/icons.qrc \
    sounds/sounds.qrc

win32:RC_ICONS = icons/ricochet_refresh.ico
OTHER_FILES += ui/qml/*
lupdate_only {
    SOURCES += ui/qml/*.qml
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
