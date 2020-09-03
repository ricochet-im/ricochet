win32-g++ {
    QMAKE_LFLAGS += -static
    QMAKE_LFLAGS += -static-libgcc
    QMAKE_LFLAGS += -static-libstdc++
    # work around issue with link time optimization bug in mingw
    # https://sourceware.org/bugzilla/show_bug.cgi?id=12762
    QMAKE_LFLAGS += -Wl,-allow-multiple-definition
}

CONFIG(debug,debug|release) {
    QMAKE_LFLAGS += -Wl,-O1
}
QMAKE_LFLAGS += -flto
