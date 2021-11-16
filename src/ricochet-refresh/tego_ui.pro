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

QMAKE_INCLUDES = $${PWD}/../qmake_includes

include($${QMAKE_INCLUDES}/artifacts.pri)
include($${QMAKE_INCLUDES}/compiler_flags.pri)
include($${QMAKE_INCLUDES}/linker_flags.pri)

TARGET = ricochet-refresh
TEMPLATE = app

QT += core gui network quick widgets

isEmpty(RICOCHET_REFRESH_VERSION) {
    VERSION = devbuild
} else {
    VERSION = $${RICOCHET_REFRESH_VERSION}
}

DEFINES += "TEGO_VERSION=$${VERSION}"

# Use CONFIG+=no-hardened to disable compiler hardening options
!CONFIG(no-hardened) {
    CONFIG += hardened
    include($${QMAKE_INCLUDES}/hardened.pri)
}

macx {
    CONFIG += bundle force_debug_plist
    QT += macextras

    # Qt 5.4 introduces a bug that breaks QMAKE_INFO_PLIST when qmake has a relative path.
    # Work around by copying Info.plist directly.
    QMAKE_INFO_PLIST = Info.plist

    icons.files = icons/ricochet_refresh.icns
    icons.path = Contents/Resources/
    QMAKE_BUNDLE_DATA += icons
}

# Create a pdb for release builds as well, to enable debugging
win32-msvc2008|win32-msvc2010 {
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF,ICF
}


# Exclude unneeded plugins from static builds
QTPLUGIN.playlistformats = -
QTPLUGIN.imageformats = -
QTPLUGIN.printsupport = -
QTPLUGIN.mediaservice = -
# Include Linux input plugins, which are missing by default, to provide complex input support. See issue #60.
unix:!macx:QTPLUGIN.platforminputcontexts = composeplatforminputcontextplugin ibusplatforminputcontextplugin

DEFINES += QT_NO_CAST_TO_ASCII

# QML
RESOURCES +=\
    $${PWD}/../libtego_ui/ui/qml/qml.qrc \
    icons/icons.qrc \
    sounds/sounds.qrc

win32:RC_ICONS = icons/ricochet_refresh.ico
OTHER_FILES += $${PWD}/../libtego_ui/ui/qml/*
lupdate_only {
    SOURCES += $${PWD}/../libtego_ui/ui/qml/*.qml
    SOURCES += $${PWD}/../libtego_ui/ui/*.cpp
    SOURCES += $${PWD}/../libtego_ui/ui/*.h
    SOURCES += $${PWD}/../libtego_ui/shims/*.cpp
    SOURCES += $${PWD}/../libtego_ui/shims/*.h
    SOURCES += $${PWD}/../libtego_ui/utils/*.cpp
    SOURCES += $${PWD}/../libtego_ui/utils/*.h
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
{
    contains(QMAKE_HOST.os,Windows):QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
    for (translation, TRANSLATIONS) {
        system($$QMAKE_LRELEASE translation/$${translation}.ts -qm translation/$${translation}.qm)
    }
}

RESOURCES += translation/embedded.qrc

CONFIG += precompile_header
PRECOMPILED_HEADER = precomp.hpp

SOURCES += main.cpp

include($${PWD}/../libtego_ui/libtego_ui.pri)
include($${PWD}/../libtego/libtego.pri)
