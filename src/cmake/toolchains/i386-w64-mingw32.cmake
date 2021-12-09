# We need some fixups to get QT working properly on i[3-6]86 platforms

set(CMAKE_SYSTEM_NAME Windows CACHE STRING "" FORCE)
set(CMAKE_SYSTEM_PROCESSOR i386 CACHE STRING "" FORCE)

set(CMAKE_C_FLAGS "-m32" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-m32" CACHE STRING "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS "-m32" CACHE STRING "" FORCE)

set(CMAKE_PREFIX_PATH "/var/tmp/dist/mingw-w64/i686-w64-mingw32/lib/" CACHE STRING "" FORCE)