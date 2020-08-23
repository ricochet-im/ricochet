unix {
    # get us onto the latest c++
    QMAKE_CXXFLAGS += --std=c++2a
    # enable c++ concepts
    QMAKE_CXXFLAGS += -fconcepts-ts
    # enable link time optimization in debug builds
    CONFIG(debug,debug|release) {
        QMAKE_CXXFLAGS += -flto -O1
        QMAKE_CFLAGS += -flto -O1
        QMAKE_LFLAGS += -flto -Wl,-O1
    }
}