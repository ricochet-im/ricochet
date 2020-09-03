unix {
    !isEmpty(OPENSSLDIR) {
        LIBS += -L$${OPENSSLDIR}/lib -lcrypto
    INCLUDEPATH += $${OPENSSLDIR}/include
    } else {
        CONFIG += link_pkgconfig
        PKGCONFIG += libcrypto
    }
}
win32 {
    win32-g++ {
        LIBS += -L$${OPENSSLDIR}/lib -lcrypto
    LIBS += -lcrypt32
    INCLUDEPATH += $${OPENSSLDIR}/include
    } else {
        LIBS += -L$${OPENSSLDIR}/lib -llibeay32
    }

    # required by openssl
    LIBS += -luser32 -lgdi32 -ladvapi32
}