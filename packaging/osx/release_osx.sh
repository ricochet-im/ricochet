#!/bin/bash

# Create and package a clean, release-mode build of the application
# Must be run from the git source directory.
# Create a .packagingrc file to define:
#  TOR_BINARY Path to tor binary to copy into bundle.
#             Should be built with --enable-static-libevent
# You can also set other environment, e.g. MAKEOPTS and PATH (for qmake)

# This script assumes Qt >= 5.4.0, for correct codesigning and macdeployqt behavior.

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
strip ricochet.app/Contents/MacOS/tor
strip ricochet.app/Contents/MacOS/ricochet

macdeployqt ricochet.app -qmldir=../src/ui/qml/
mv ricochet.app Ricochet.app

# Code signing, if CODESIGN_ID is defined
if [ ! -z "$CODESIGN_ID" ]; then
    codesign --verbose --sign "$CODESIGN_ID" --deep Ricochet.app
fi

hdiutil create Ricochet.dmg -srcfolder Ricochet.app -format UDZO -volname Ricochet

echo "---------"

otool -L Ricochet.app/Contents/MacOS/ricochet
otool -L Ricochet.app/Contents/MacOS/tor

codesign -vvvv -d Ricochet.app
spctl -vvvv --assess --type execute Ricochet.app

echo
echo "Output: ./build/Ricochet.dmg"

