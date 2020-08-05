include($${PWD}/../qmake_includes/artifacts.pri)

TEMPLATE = lib
TARGET = tego

CONFIG += staticlib

HEADERS =\
    include/tego/tego.h\
    include/tego/logger.hpp

SOURCES =\
    source/libtego.cpp\
    source/logger.cpp\

SOURCES +=\
    $${PWD}/../extern/fmt/src/format.cc\
    $${PWD}/../extern/fmt/src/os.cc

INCLUDEPATH +=\
    include\
    $${PWD}/../extern/fmt/include