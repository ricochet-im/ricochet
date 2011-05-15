# Torsion - http://torsionim.org/
# Copyright (C) 2010, John Brooks <john.brooks@dereferenced.net>
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

TARGET = Torsion
TEMPLATE = app
QT += core gui network declarative

CONFIG(release,debug|release):DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT

unix:!macx {
    target.path = /usr/bin
    shortcut.path = /usr/share/applications
    shortcut.files = src/Torsion.desktop
    INSTALLS += target shortcut

    exists(tor) {
        message(Adding bundled Tor to installations)
        bundletor.path = /usr/lib/torsion/tor/
        bundletor.files = tor/*
        INSTALLS += bundletor
        DEFINES += BUNDLED_TOR_PATH=\\\"/usr/lib/torsion/tor/\\\"
    }
} else:macx {
    CONFIG += bundle

    exists(tor) {
        # Copy the entire tor/ directory, which should contain tor/tor (the binary itself)
        QMAKE_POST_LINK += cp -R $${_PRO_FILE_PWD_}/tor $${OUT_PWD}/$${TARGET}.app/Contents/MacOS/;
    }
}

CONFIG += debug_and_release
QMAKE_RESOURCE_FLAGS += -no-compress

# Create a pdb for release builds as well, to enable debugging
win32-msvc2008|win32-msvc2010 {
    QMAKE_CXXFLAGS_RELEASE += /Zi
    QMAKE_LFLAGS_RELEASE += /DEBUG /OPT:REF,ICF
}

INCLUDEPATH += src

unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += libcrypto # Using libcrypto instead of openssl to avoid needlessly linking libssl
}
win32 {
    isEmpty(OPENSSLDIR):error(You must pass OPENSSLDIR=path/to/openssl to qmake on this platform)
    INCLUDEPATH += $${OPENSSLDIR}/include
    LIBS += -L$${OPENSSLDIR}/lib -llibeay32

    # required by openssl
    LIBS += -lUser32 -lGdi32 -ladvapi32
}
macx:LIBS += -lcrypto

DEFINES += QT_NO_CAST_FROM_ASCII QT_NO_CAST_TO_ASCII

SOURCES += src/main.cpp \
    src/ui/MainWindow.cpp \
    src/ui/ChatWidget.cpp \
    src/ContactsModel.cpp \
    src/tor/TorControlManager.cpp \
    src/tor/TorControlSocket.cpp \
    src/tor/TorControlCommand.cpp \
    src/tor/ProtocolInfoCommand.cpp \
    src/tor/AuthenticateCommand.cpp \
    src/tor/SetConfCommand.cpp \
    src/utils/StringUtil.cpp \
    src/core/ContactsManager.cpp \
    src/core/ContactUser.cpp \
    src/protocol/ProtocolCommand.cpp \
    src/protocol/ProtocolManager.cpp \
    src/protocol/PingCommand.cpp \
    src/ui/ContactInfoPage.cpp \
    src/protocol/IncomingSocket.cpp \
    src/protocol/ChatMessageCommand.cpp \
    src/protocol/CommandHandler.cpp \
    src/protocol/CommandDataParser.cpp \
    src/tor/GetConfCommand.cpp \
    src/utils/DateUtil.cpp \
    src/tor/TorServiceTest.cpp \
    src/tor/HiddenService.cpp \
    src/ui/torconfig/TorConfigWizard.cpp \
    src/ui/torconfig/IntroPage.cpp \
    src/ui/torconfig/ManualConfigPage.cpp \
    src/protocol/ProtocolSocket.cpp \
    src/ui/torconfig/TorConnTestWidget.cpp \
    src/utils/PaintUtil.cpp \
    src/utils/CryptoKey.cpp \
    src/utils/SecureRNG.cpp \
    src/protocol/ContactRequestClient.cpp \
    src/protocol/ContactRequestServer.cpp \
    src/core/OutgoingContactRequest.cpp \
    src/ui/ContactAddDialog.cpp \
    src/ui/FancyTextEdit.cpp \
    src/core/IncomingRequestManager.cpp \
    src/core/ContactIDValidator.cpp \
    src/core/NicknameValidator.cpp \
    src/ui/NotificationWidget.cpp \
    src/ui/ContactRequestDialog.cpp \
    src/protocol/GetSecretCommand.cpp \
    src/ui/EditableLabel.cpp \
    src/ui/ContactIDWidget.cpp \
    src/tor/autoconfig/VidaliaConfigManager.cpp \
    src/ui/torconfig/VidaliaConfigPage.cpp \
    src/ui/torconfig/VidaliaExitWidget.cpp \
    src/ui/torconfig/VidaliaStartWidget.cpp \
    src/ui/torconfig/VidaliaTestWidget.cpp \
    src/core/UserIdentity.cpp \
    src/core/IdentityManager.cpp \
    src/ui/IdentityInfoPage.cpp \
    src/tor/autoconfig/BundledTorManager.cpp \
    src/utils/OSUtil.cpp \
    src/ui/ChatTextWidget.cpp \
    src/utils/AppSettings.cpp \
    src/ui/ExpandingTextEdit.cpp \
    src/ui/ChatTextInput.cpp \
    src/ui/PopoutManager.cpp \
    src/ui/UIHelper.cpp

HEADERS += src/ui/MainWindow.h \
    src/ui/ChatWidget.h \
    src/ContactsModel.h \
    src/tor/TorControlManager.h \
    src/tor/TorControlSocket.h \
    src/tor/TorControlCommand.h \
    src/tor/ProtocolInfoCommand.h \
    src/tor/AuthenticateCommand.h \
    src/tor/SetConfCommand.h \
    src/utils/StringUtil.h \
    src/core/ContactsManager.h \
    src/core/ContactUser.h \
    src/protocol/ProtocolCommand.h \
    src/protocol/ProtocolManager.h \
    src/protocol/PingCommand.h \
    src/ui/ContactInfoPage.h \
    src/protocol/IncomingSocket.h \
    src/main.h \
    src/protocol/ChatMessageCommand.h \
    src/protocol/CommandHandler.h \
    src/protocol/CommandDataParser.h \
    src/tor/GetConfCommand.h \
    src/utils/DateUtil.h \
    src/tor/TorServiceTest.h \
    src/tor/HiddenService.h \
    src/ui/torconfig/TorConfigWizard.h \
    src/ui/torconfig/IntroPage.h \
    src/ui/torconfig/ManualConfigPage.h \
    src/protocol/ProtocolSocket.h \
    src/ui/torconfig/TorConnTestWidget.h \
    src/utils/PaintUtil.h \
    src/utils/CryptoKey.h \
    src/utils/SecureRNG.h \
    src/protocol/ContactRequestClient.h \
    src/protocol/ContactRequestServer.h \
    src/core/OutgoingContactRequest.h \
    src/ui/ContactAddDialog.h \
    src/ui/FancyTextEdit.h \
    src/core/IncomingRequestManager.h \
    src/core/ContactIDValidator.h \
    src/core/NicknameValidator.h \
    src/ui/NotificationWidget.h \
    src/ui/ContactRequestDialog.h \
    src/protocol/GetSecretCommand.h \
    src/ui/EditableLabel.h \
    src/ui/ContactIDWidget.h \
    src/tor/autoconfig/VidaliaConfigManager.h \
    src/ui/torconfig/VidaliaConfigPage.h \
    src/ui/torconfig/VidaliaExitWidget.h \
    src/ui/torconfig/VidaliaStartWidget.h \
    src/ui/torconfig/VidaliaTestWidget.h \
    src/core/UserIdentity.h \
    src/core/IdentityManager.h \
    src/ui/IdentityInfoPage.h \
    src/tor/autoconfig/BundledTorManager.h \
    src/utils/OSUtil.h \
    src/ui/ChatTextWidget.h \
    src/utils/AppSettings.h \
    src/ui/ExpandingTextEdit.h \
    src/ui/ChatTextInput.h \
    src/ui/PopoutManager.h \
    src/ui/PageSwitcherBase.h \
    src/ui/UIHelper.h

RESOURCES += res/resources.qrc \
    translation/embedded.qrc \
    src/ui/qml/qml.qrc

TRANSLATIONS = translation/torsion.ts

OTHER_FILES += \
    src/ui/qml/main.qml \
    src/ui/qml/TorsionToolBar.qml \
    src/ui/qml/switcher.js \
    src/ui/qml/PopoutItem.qml \
    src/ui/qml/PopoutClickArea.qml \
    src/ui/qml/PageSwitcher.qml \
    src/ui/qml/ContactPage.qml \
    src/ui/qml/ContactListItem.qml \
    src/ui/qml/ContactListGroup.qml \
    src/ui/qml/ContactList.qml \
    src/ui/qml/ContactGroupView.qml \
    src/ui/qml/ChatArea.qml \
    src/ui/qml/Avatar.qml
