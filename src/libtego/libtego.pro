include($${PWD}/../qmake_includes/artifacts.pri)

TEMPLATE = lib
TARGET = tego

CONFIG += staticlib

HEADERS =\
    include/libtego.h

SOURCES =\
    source/libtego.cpp