include(../../hardened.pri)
QMAKE_LFLAGS += $$HARDENED_MINGW_64ASLR_FLAGS
SOURCES += test.cpp
