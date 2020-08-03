include($${PWD}/../qmake_includes/artifacts.pri)

TEMPLATE = lib
TARGET = tego

CONFIG += staticlib

HEADS =\
    include/libtego.h

SOURCES =\
    source/libtego.cpp