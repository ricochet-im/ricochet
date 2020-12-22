# get us onto the latest c++
QMAKE_CXXFLAGS += --std=c++2a
# enable link time optimization
QMAKE_CFLAGS += -flto
QMAKE_CXXFLAGS += -flto
CONFIG(debug,debug|release) {
    QMAKE_CFLAGS += -O1
    QMAKE_CXXFLAGS += -O1
}

CONFIG(release,debug|release):DEFINES += QT_NO_DEBUG_OUTPUT QT_NO_WARNING_OUTPUT