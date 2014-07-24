#!/bin/bash

# Create and package a clean, release-mode build of the application
# Must be run from the git source directory.
# Create a .packagingrc file to define:
#  TOR_BINARY Path to tor binary to copy into bundle.
#             Should be built with --enable-static-libevent
# You can also set other environment, e.g. MAKEOPTS and PATH (for qmake)

set -e

if [ ! -d .git ] || [ ! -f ricochet.pro ]; then
    echo "Must be run from source directory"
    exit 1
fi

. .packagingrc

if [ -z "$TOR_BINARY" ] || [ ! -f "$TOR_BINARY" ]; then
    echo "Missing TOR_BINARY: $TOR_BINARY"
    exit 1
fi

rm -r build || true
mkdir build
cd build

qmake CONFIG+=release ..
make

cp "$TOR_BINARY" ricochet.app/Contents/MacOS/

macdeployqt ricochet.app -qmldir=../src/ui/qml/
# Workaround macdeployqt bug
rm -r ricochet.app/Contents/Resources/qml/{im,ContactWindow,qml}
mv ricochet.app Ricochet.app

# Code signing, if CODESIGN_ID is defined
if [ ! -z "$CODESIGN_ID" ]; then
    # Workaround https://bugreports.qt-project.org/browse/QTBUG-23268, credit to Sarah Jane Smith
    QT_FMWKS="QtCore QtGui QtPrintSupport QtQuick QtQml QtWidgets QtNetwork"
    QT_LIB_DIR="`qmake -query QT_INSTALL_LIBS`"

    for FMWK in $QT_FMWKS; do
        FMWK_PATH="Ricochet.app/Contents/Frameworks/${FMWK}.framework"
        mv -v "${FMWK_PATH}/Resources" "${FMWK_PATH}/Versions/5/."
        (cd "${FMWK_PATH}" && ln -s "Versions/5/Resources" "Resources")
        (cd "${FMWK_PATH}/Versions" && ln -s "5" "Current")
        cp "${QT_LIB_DIR}/${FMWK}.framework/Contents/Info.plist" "${FMWK_PATH}/Resources/Info.plist"
        perl -pi -e "s/${FMWK}_debug/${FMWK}/" "${FMWK_PATH}/Resources/Info.plist"
    done

    for FMWK in $QT_FMWKS; do
        FMWK_PATH="Ricochet.app/Contents/Frameworks/${FMWK}.framework"
        /usr/bin/codesign --verbose --sign "$CODESIGN_ID" "${FMWK_PATH}/Versions/5"
    done

    for PLGN in $(find Ricochet.app -name "*.dylib"); do
        /usr/bin/codesign --verbose --sign "$CODESIGN_ID" ${PLGN}
    done

    codesign --verbose --sign "$CODESIGN_ID" Ricochet.app/Contents/MacOS/tor
    codesign --verbose --sign "$CODESIGN_ID" Ricochet.app
fi

hdiutil create Ricochet.dmg -srcfolder Ricochet.app -format UDZO -volname Ricochet

echo "---------"

otool -L Ricochet.app/Contents/MacOS/ricochet
otool -L Ricochet.app/Contents/MacOS/tor

codesign -vvvv -d Ricochet.app
spctl -vvvv --assess --type execute Ricochet.app

echo
echo "Output: ./build/Ricochet.dmg"

