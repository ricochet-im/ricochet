include($${PWD}/../qmake_includes/artifacts.pri)

TEMPLATE = subdirs
CONFIG += ordered

SUBDIRS = \
    tst_cryptokey \
    tst_contactidvalidator \
