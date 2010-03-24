#-------------------------------------------------
#
# Project created by QtCreator 2010-03-24T15:47:07
#
#-------------------------------------------------

QT       += core gui

TARGET = TorIM
TEMPLATE = app

INCLUDEPATH += src

SOURCES += src/main.cpp\
        src/ui/MainWindow.cpp \
    src/ui/ChatWidget.cpp \
    src/ContactsModel.cpp

HEADERS  += src/ui/MainWindow.h \
    src/ui/ChatWidget.h \
    src/ContactsModel.h

OTHER_FILES += \
    res/user--plus.png

RESOURCES += \
    res/resources.qrc
