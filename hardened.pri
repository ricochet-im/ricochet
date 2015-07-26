load(configure)
# Define common variables; these are used by config tests _and_ the actual build
HARDENED_SANITIZE_FLAGS = -fsanitize=address
# Also: -fsanitize=undefined -fsanitize=integer-divide-by-zero -fvtable-verify=std
HARDENED_SANITIZE_MORE_FLAGS = -fsanitize=bounds -fsanitize=vptr -fsanitize=object-size -fsanitize=alignment -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow
HARDENED_STACK_PROTECTOR_STRONG_FLAGS = -fstack-protector-strong
HARDENED_STACK_PROTECTOR_FLAGS = -fstack-protector --param=ssp-buffer-size=4

# Run tests and apply options where possible
CONFIG(hardened) {
    HARDEN_FLAGS = -fPIC
    qtCompileTest(sanitize):HARDEN_FLAGS += $$HARDENED_SANITIZE_FLAGS
    qtCompileTest(sanitize-more):HARDEN_FLAGS += $$HARDENED_SANITIZE_MORE_FLAGS
    qtCompileTest(stack-protector-strong) {
        HARDEN_FLAGS += $$HARDENED_STACK_PROTECTOR_STRONG_FLAGS
    } else {
        qtCompileTest(stack-protector):HARDEN_FLAGS += $$HARDENED_STACK_PROTECTOR_FLAGS
    }

    QMAKE_CXXFLAGS *= $$HARDEN_FLAGS
    QMAKE_LFLAGS *= $$HARDEN_FLAGS

    # _FORTIFY_SOURCE requires -O, so only use on release builds
    CONFIG(release,debug|release):QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2
    # Linux specific
    unix:!macx:QMAKE_LFLAGS *= -pie -Wl,-z,relro,-z,now
}
