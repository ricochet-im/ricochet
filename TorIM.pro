# -------------------------------------------------
# Project created by QtCreator 2010-03-24T15:47:07
# -------------------------------------------------
QT += core \
    gui \
    network
TARGET = TorIM
TEMPLATE = app
INCLUDEPATH += src
SOURCES += src/main.cpp \
    src/ui/MainWindow.cpp \
    src/ui/ChatWidget.cpp \
    src/ContactsModel.cpp \
    src/ui/ContactItemDelegate.cpp \
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
    src/ui/ContactsView.cpp \
    src/protocol/IncomingSocket.cpp \
    src/protocol/ChatMessageCommand.cpp \
    src/protocol/CommandHandler.cpp \
    src/protocol/CommandDataParser.cpp \
    src/ui/HomeScreen.cpp \
    src/tor/GetConfCommand.cpp \
    src/ui/HomeContactWidget.cpp \
    src/utils/DateUtil.cpp \
    src/tor/TorServiceTest.cpp \
    src/tor/HiddenService.cpp
HEADERS += src/ui/MainWindow.h \
    src/ui/ChatWidget.h \
    src/ContactsModel.h \
    src/ui/ContactItemDelegate.h \
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
    src/ui/ContactsView.h \
    src/protocol/IncomingSocket.h \
    src/main.h \
    src/protocol/ChatMessageCommand.h \
    src/protocol/CommandHandler.h \
    src/protocol/CommandDataParser.h \
    src/ui/HomeScreen.h \
    src/tor/GetConfCommand.h \
    src/ui/HomeContactWidget.h \
    src/utils/DateUtil.h \
    src/tor/TorServiceTest.h \
    src/tor/HiddenService.h
RESOURCES += res/resources.qrc \
    translation/embedded.qrc
TRANSLATIONS = translation/torim.ts
