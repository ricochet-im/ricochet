include($${PWD}/qmake_includes/artifacts.pri)

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS = \
    libtego \
    libtego_ui \
    tego_ui \
    tests \

