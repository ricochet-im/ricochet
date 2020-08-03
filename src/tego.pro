include($${PWD}/qmake_includes/artifacts.pri)

TEMPLATE = subdirs
CONFIG += ordered
SUBDIRS =\
    libtego \
    tego-ui

tego-ui.depends = libtego