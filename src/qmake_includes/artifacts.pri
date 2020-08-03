# specify the DESTDIR for final binary and intermediate build files
release:DESTDIR = release
debug:DESTDIR = debug

# artifacts go under hidden dirs in DESTDIR
OBJECTS_DIR = $${DESTDIR}/.obj
MOC_DIR     = $${DESTDIR}/.moc
RCC_DIR     = $${DESTDIR}/.rcc
UI_DIR      = $${DESTDIR}/.ui