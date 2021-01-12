win32-g++ {
    QMAKE_LFLAGS += -static
    QMAKE_LFLAGS += -static-libgcc
    QMAKE_LFLAGS += -static-libstdc++
}

# link time optimization for non-windows targets
!win32-g++ {
    CONFIG(debug,debug|release) {
        QMAKE_LFLAGS += -Wl,-O1
    }
    QMAKE_LFLAGS += -flto
}
