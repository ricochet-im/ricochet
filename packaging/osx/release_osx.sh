#!/bin/bash

# Create and package a clean, release-mode build of the application
# Must be run from the git source directory.
# Create a .packagingrc file to define:
#  TOR_BINARY Path to tor binary to copy into bundle.
#             Should be built with --enable-static-libevent
# You can also set other environment, e.g. MAKEOPTS and PATH (for qmake)

set -e

if [ ! -d .git ] || [ ! -f Torsion.pro ]; then
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

cp "$TOR_BINARY" Torsion.app/Contents/MacOS/

macdeployqt Torsion.app -qmldir=../src/ui/qml/ -dmg
# Workaround macdeployqt bug
rm -r Torsion.app/Contents/Resources/qml/org

echo "---------"

otool -L Torsion.app/Contents/MacOS/Torsion
otool -L Torsion.app/Contents/MacOS/tor

echo
echo "Output: ./build/Torsion.dmg"

