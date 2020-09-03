# get us onto the latest c++
QMAKE_CXXFLAGS += --std=c++2a
# enable link time optimization
QMAKE_CFLAGS += -flto
QMAKE_CXXFLAGS += -flto