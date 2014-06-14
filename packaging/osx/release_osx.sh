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
rm -r ricochet.app/Contents/Resources/qml/org

hdiutil create Ricochet.dmg -srcfolder ricochet.app -format UDZO -volname Ricochet

echo "---------"

otool -L ricochet.app/Contents/MacOS/ricochet
otool -L ricochet.app/Contents/MacOS/tor

echo
echo "Output: ./build/Ricochet.dmg"

