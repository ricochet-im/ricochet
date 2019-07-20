include(../tests.pri)

SOURCES += tst_cryptokey.cpp \
    $${SRC}/utils/CryptoKey.cpp \
    $${SRC}/utils/SecureRNG.cpp

unix {
    !isEmpty(OPENSSLDIR) {
        INCLUDEPATH += $${OPENSSLDIR}/include
        LIBS += -L$${OPENSSLDIR}/lib -lcrypto
    } else {
        CONFIG += link_pkgconfig
        PKGCONFIG += libcrypto
    }
}
win32 {
    isEmpty(OPENSSLDIR):error(You must pass OPENSSLDIR=path/to/openssl to qmake on this platform)
    INCLUDEPATH += $${OPENSSLDIR}/include
    LIBS += -L$${OPENSSLDIR}/lib -llibeay32

    # required by openssl
    LIBS += -lUser32 -lGdi32 -ladvapi32
}
macx:LIBS += -lcrypto
