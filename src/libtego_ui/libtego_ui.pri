INCLUDEPATH += $${PWD}

unix {
    !isEmpty(OPENSSLDIR) {
        LIBS += -L$${OPENSSLDIR}/lib -lcrypto
    } else {
        CONFIG += link_pkgconfig
        PKGCONFIG += libcrypto
    }
}
win32 {
    win32-g++ {
        LIBS += -L$${OPENSSLDIR}/lib -lcrypto
    } else {
        LIBS += -L$${OPENSSLDIR}/lib -llibeay32
    }

    # required by openssl
    LIBS += -luser32 -lgdi32 -ladvapi32
}

include($${PWD}/../qmake_includes/protobuf.pri)

LIBS += -L$${DESTDIR}/../libtego_ui -ltego_ui
