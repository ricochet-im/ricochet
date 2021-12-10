set(CMAKE_SYSTEM_NAME Darwin CACHE STRING "" FORCE)
set(CMAKE_SYSTEM_PROCESSOR x86_64 CACHE STRING "" FORCE)

set(CMAKE_C_COMPILER_TARGET "x86_64-apple-darwin")
set(CMAKE_CXX_COMPILER_TARGET "x86_64-apple-darwin")
set(CMAKE_ASM_COMPILER_TARGET "x86_64-apple-darwin")

set(CMAKE_PREFIX_PATH "/var/tmp/dist/macosx-toolchain/clang/lib/;/var/tmp/dist/macosx-toolchain/MacOSX10.15.sdk/System/Library/Frameworks" CACHE STRING "" FORCE)
