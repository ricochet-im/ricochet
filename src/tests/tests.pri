TEMPLATE = app
QT += testlib core gui network quick widgets
CONFIG -= app_bundle
CONFIG += testcase

QMAKE_INCLUDES = $${PWD}/../qmake_includes

include($${QMAKE_INCLUDES}/artifacts.pri)
include($${QMAKE_INCLUDES}/compiler_flags.pri)

include($${PWD}/../libtego_ui/libtego_ui.pri)
include($${PWD}/../libtego/libtego.pri)