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
    src/core/TorControlManager.cpp \
    src/core/TorControlSocket.cpp

HEADERS  += src/ui/MainWindow.h \
    src/ui/ChatWidget.h \
    src/ContactsModel.h \
    src/ui/ContactItemDelegate.h \
    src/core/TorControlManager.h \
    src/core/TorControlSocket.h

OTHER_FILES += \
    res/user--plus.png

RESOURCES += \
    res/resources.qrc
