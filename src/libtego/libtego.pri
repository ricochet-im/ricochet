INCLUDEPATH +=\
    $${PWD}/include\
    $${PWD}/../extern/fmt/include

LIBS += -L$${DESTDIR}/../libtego -ltego

QMAKE_INCLUDES = $${PWD}/../qmake_includes
include($${QMAKE_INCLUDES}/openssl.pri)