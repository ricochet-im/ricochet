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

include($${PWD}/qmake_includes/artifacts.pri)

TARGET = ricochet-refresh
TEMPLATE = app
QT += core gui network quick widgets

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
        target.path = /usr/bin
        shortcut.path = /usr/share/applications
        shortcut.files = tego-ui/ricochet.desktop
        icon.path = /usr/share/icons/hicolor/48x48/apps/
        icon.files = tego-ui/icons/ricochet_refresh.png
        scalable_icon.path = /usr/share/icons/hicolor/scalable/apps/
        scalable_icon.files = tego-ui/icons/ricochet_refresh.svg
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
        QMAKE_INFO_PLIST = tego-ui/Info.plist
    } else:equals(QT_MAJOR_VERSION,5):lessThan(QT_MINOR_VERSION,4) {
        QMAKE_INFO_PLIST = tego-ui/Info.plist
    } else {
        CONFIG += no_plist
        QMAKE_POST_LINK += cp $${_PRO_FILE_PWD_}/tego-ui/Info.plist $${OUT_PWD}/$${TARGET}.app/Contents/;
    }

    exists(tor) {
        # Copy the entire tor/ directory, which should contain tor/tor (the binary itself)
        QMAKE_POST_LINK += cp -R $${_PRO_FILE_PWD_}/tor $${OUT_PWD}/$${TARGET}.app/Contents/MacOS/;
    }

    icons.files = icons/ricochet_refresh.icns
    icons.path = Contents/Resources/
    QMAKE_BUNDLE_DATA += icons
}

CONFIG += debug_and_release

# Create a pdb for release builds as well, to enable debugging
win32-msvc2008|win32-msvc2010 {
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF,ICF
}

INCLUDEPATH += tego-ui

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

# Exclude unneeded plugins from static builds
QTPLUGIN.playlistformats = -
QTPLUGIN.imageformats = -
QTPLUGIN.printsupport = -
QTPLUGIN.mediaservice = -
# Include Linux input plugins, which are missing by default, to provide complex input support. See issue #60.
unix:!macx:QTPLUGIN.platforminputcontexts = composeplatforminputcontextplugin ibusplatforminputcontextplugin

DEFINES += QT_NO_CAST_TO_ASCII

SOURCES += tego-ui/main.cpp \
    tego-ui/ui/MainWindow.cpp \
    tego-ui/ui/ContactsModel.cpp \
    tego-ui/tor/TorControl.cpp \
    tego-ui/tor/TorControlSocket.cpp \
    tego-ui/tor/TorControlCommand.cpp \
    tego-ui/tor/ProtocolInfoCommand.cpp \
    tego-ui/tor/AuthenticateCommand.cpp \
    tego-ui/tor/SetConfCommand.cpp \
    tego-ui/tor/AddOnionCommand.cpp \
    tego-ui/utils/StringUtil.cpp \
    tego-ui/core/ContactsManager.cpp \
    tego-ui/core/ContactUser.cpp \
    tego-ui/tor/GetConfCommand.cpp \
    tego-ui/tor/HiddenService.cpp \
    tego-ui/utils/CryptoKey.cpp \
    tego-ui/utils/SecureRNG.cpp \
    tego-ui/core/OutgoingContactRequest.cpp \
    tego-ui/core/IncomingRequestManager.cpp \
    tego-ui/core/ContactIDValidator.cpp \
    tego-ui/core/UserIdentity.cpp \
    tego-ui/core/IdentityManager.cpp \
    tego-ui/core/ConversationModel.cpp \
    tego-ui/tor/TorProcess.cpp \
    tego-ui/tor/TorManager.cpp \
    tego-ui/tor/TorSocket.cpp \
    tego-ui/ui/LinkedText.cpp \
    tego-ui/utils/Settings.cpp \
    tego-ui/utils/PendingOperation.cpp \
    tego-ui/ui/LanguagesModel.cpp

HEADERS += tego-ui/ui/MainWindow.h \
    tego-ui/ui/ContactsModel.h \
    tego-ui/tor/TorControl.h \
    tego-ui/tor/TorControlSocket.h \
    tego-ui/tor/TorControlCommand.h \
    tego-ui/tor/ProtocolInfoCommand.h \
    tego-ui/tor/AuthenticateCommand.h \
    tego-ui/tor/SetConfCommand.h \
    tego-ui/tor/AddOnionCommand.h \
    tego-ui/utils/StringUtil.h \
    tego-ui/core/ContactsManager.h \
    tego-ui/core/ContactUser.h \
    tego-ui/tor/GetConfCommand.h \
    tego-ui/tor/HiddenService.h \
    tego-ui/utils/CryptoKey.h \
    tego-ui/utils/SecureRNG.h \
    tego-ui/core/OutgoingContactRequest.h \
    tego-ui/core/IncomingRequestManager.h \
    tego-ui/core/ContactIDValidator.h \
    tego-ui/core/UserIdentity.h \
    tego-ui/core/IdentityManager.h \
    tego-ui/core/ConversationModel.h \
    tego-ui/tor/TorProcess.h \
    tego-ui/tor/TorProcess_p.h \
    tego-ui/tor/TorManager.h \
    tego-ui/tor/TorSocket.h \
    tego-ui/ui/LinkedText.h \
    tego-ui/utils/Settings.h \
    tego-ui/utils/PendingOperation.h \
    tego-ui/ui/LanguagesModel.h

SOURCES += tego-ui/protocol/Channel.cpp \
    tego-ui/protocol/ControlChannel.cpp \
    tego-ui/protocol/Connection.cpp \
    tego-ui/protocol/OutboundConnector.cpp \
    tego-ui/protocol/AuthHiddenServiceChannel.cpp \
    tego-ui/protocol/ChatChannel.cpp \
    tego-ui/protocol/ContactRequestChannel.cpp

HEADERS += tego-ui/protocol/Channel.h \
    tego-ui/protocol/Channel_p.h \
    tego-ui/protocol/ControlChannel.h \
    tego-ui/protocol/Connection.h \
    tego-ui/protocol/Connection_p.h \
    tego-ui/protocol/OutboundConnector.h \
    tego-ui/protocol/AuthHiddenServiceChannel.h \
    tego-ui/protocol/ChatChannel.h \
    tego-ui/protocol/ContactRequestChannel.h

# fmt lib
SOURCE +=\
    tego-ui/fmt/format.cc \
    tego-ui/fmt/posix.cc

HEADERS +=\
    tego-ui/fmt/core.h \
    tego-ui/fmt/format.h \
    tego-ui/fmt/format-inl.h \
    tego-ui/fmt/ostream.h \
    tego-ui/fmt/posix.h \
    tego-ui/fmt/printf.h \
    tego-ui/fmt/ranges.h \
    tego-ui/fmt/time.h

# custom
HEADERS +=\
    tego-ui/logger.hpp

SOURCES +=\
    tego-ui/logger.cpp

include(protobuf.pri)
PROTOS += tego-ui/protocol/ControlChannel.proto \
    tego-ui/protocol/AuthHiddenService.proto \
    tego-ui/protocol/ChatChannel.proto \
    tego-ui/protocol/ContactRequestChannel.proto

# QML
RESOURCES += tego-ui/ui/qml/qml.qrc \
    tego-ui/icons/icons.qrc \
    tego-ui/sounds/sounds.qrc

win32:RC_ICONS = tego-ui/icons/ricochet_refresh.ico
OTHER_FILES += tego-ui/ui/qml/*
lupdate_only {
    SOURCES += tego-ui/ui/qml/*.qml
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
        system($$QMAKE_LRELEASE tego-ui/translation/$${translation}.ts -qm tego-ui/translation/$${translation}.qm)
    }
}

RESOURCES += tego-ui/translation/embedded.qrc
