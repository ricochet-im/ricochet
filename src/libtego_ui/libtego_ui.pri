INCLUDEPATH += $${PWD}

LIBS += -L$${DESTDIR}/../libtego_ui -ltego_ui

macx {
    QT += macextras
}
QMAKE_INCLUDES = $${PWD}/../qmake_includes
include($${QMAKE_INCLUDES}/protobuf.pri)
include($${QMAKE_INCLUDES}/openssl.pri)