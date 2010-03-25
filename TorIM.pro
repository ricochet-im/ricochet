#-------------------------------------------------
#
# Project created by QtCreator 2010-03-24T15:47:07
#
#-------------------------------------------------

QT       += core gui network

TARGET = TorIM
TEMPLATE = app

INCLUDEPATH += src

SOURCES += src/main.cpp\
        src/ui/MainWindow.cpp \
    src/ui/ChatWidget.cpp \
    src/ContactsModel.cpp \
    src/ui/ContactItemDelegate.cpp \
    src/tor/TorControlManager.cpp \
    src/tor/TorControlSocket.cpp \
    src/tor/TorControlCommand.cpp \
    src/tor/ProtocolInfoCommand.cpp \
    src/tor/AuthenticateCommand.cpp

HEADERS  += src/ui/MainWindow.h \
    src/ui/ChatWidget.h \
    src/ContactsModel.h \
    src/ui/ContactItemDelegate.h \
    src/tor/TorControlManager.h \
    src/tor/TorControlSocket.h \
    src/tor/TorControlCommand.h \
    src/tor/ProtocolInfoCommand.h \
    src/tor/AuthenticateCommand.h

OTHER_FILES += \
    res/user--plus.png

RESOURCES += \
    res/resources.qrc
