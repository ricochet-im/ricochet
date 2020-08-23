# specify the DESTDIR for final binary and intermediate build files

CONFIG += debug_and_release

CONFIG(release, release|debug) {
    DESTDIR = $${PWD}/../../build/release/$${TARGET}
}
CONFIG(debug, release|debug) {
    DESTDIR = $${PWD}/../../build/debug/$${TARGET}
}

# artifacts go under hidden dirs in DESTDIR
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR     = $${DESTDIR}/.moc
RCC_DIR     = $${DESTDIR}/.rcc
UI_DIR      = $${DESTDIR}/.ui
