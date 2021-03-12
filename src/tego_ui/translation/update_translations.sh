#!/bin/sh

# see https://doc.qt.io/qt-5/qtquick-internationalization.html for info
# on annotating strings for translation

# this command parses the source and populates our ts files with new
# translation strings
lupdate-pro ../tego_ui.pro -ts *.ts
