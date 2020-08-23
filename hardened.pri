load(configure)
# Define common variables; these are used by config tests _and_ the actual build

# Supported in gcc 4.8+
HARDENED_SANITIZE_FLAGS = -fsanitize=address
# Supported in gcc 4.9+
HARDENED_SANITIZE_UBSAN_FLAGS = -fsanitize=undefined -fsanitize=integer-divide-by-zero -fsanitize=bounds -fsanitize=alignment -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize-recover
# Supported in gcc 5.0+
HARDENED_SANITIZE_UBSAN_MORE_FLAGS = -fsanitize=vptr -fsanitize=object-size
# vtable-verify requires some OS support; see https://bugzilla.novell.com/show_bug.cgi?id=877239
HARDENED_VTABLE_VERIFY_FLAGS = -fvtable-verify=std

HARDENED_STACK_PROTECTOR_STRONG_FLAGS = -fstack-protector-strong
HARDENED_STACK_PROTECTOR_FLAGS = -fstack-protector --param=ssp-buffer-size=4

HARDENED_MINGW_64ASLR_FLAGS = -Wl,--dynamicbase -Wl,--high-entropy-va


# Run tests and apply options where possible
CONFIG(hardened) {
    # mingw is always PIC, and complains about the flag
    !mingw:HARDEN_FLAGS = -fPIC

    CONFIG(debug,debug|release): qtCompileTest(sanitize):HARDEN_FLAGS += $$HARDENED_SANITIZE_FLAGS
    qtCompileTest(sanitize-ubsan):HARDEN_FLAGS += $$HARDENED_SANITIZE_UBSAN_FLAGS
    qtCompileTest(sanitize-ubsan-more):HARDEN_FLAGS += $$HARDENED_SANITIZE_UBSAN_MORE_FLAGS
    qtCompileTest(vtable-verify):HARDEN_FLAGS += $$HARDENED_VTABLE_VERIFY_FLAGS

    qtCompileTest(stack-protector-strong) {
        HARDEN_FLAGS += $$HARDENED_STACK_PROTECTOR_STRONG_FLAGS
    } else {
        qtCompileTest(stack-protector):HARDEN_FLAGS += $$HARDENED_STACK_PROTECTOR_FLAGS
    }

    mingw {
        qtCompileTest(mingw-64aslr):QMAKE_LFLAGS *= $$HARDENED_MINGW_64ASLR_FLAGS
        QMAKE_LFLAGS *= -Wl,--nxcompat -Wl,--dynamicbase
    }

    QMAKE_CXXFLAGS *= $$HARDEN_FLAGS
    QMAKE_LFLAGS *= $$HARDEN_FLAGS

    # _FORTIFY_SOURCE requires -O, so only use on release builds
    CONFIG(release,debug|release):QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2
    # Linux specific
    unix:!macx:QMAKE_LFLAGS *= -pie -Wl,-z,relro,-z,now
}
