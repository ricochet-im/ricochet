#!/bin/bash

# Create and package a clean, release-mode build of the application
# Install `create-dmg` with `brew install create-dmg` or from source: https://github.com/create-dmg/create-dmg
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

/usr/local/opt/qt5/bin/qmake OPENSSLDIR=/usr/local/opt/openssl/ CONFIG+=release ..
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

# Since create-dmg does not clobber, be sure to delete previous DMG
[[ -f Ricochet.dmg ]] && rm Ricochet.dmg

# Creates source folder required by create-dmg
# [[ -f osx-dmg-source ]] && rm -rf osx-dmg-source
mkdir osx-dmg-source
cp -r Ricochet.app osx-dmg-source/Ricochet.app

# Create "fancy" DMG with shortcut to `Applications` and instruction to user. (See: https://github.com/create-dmg/create-dmg) 
create-dmg --window-size 540 380 --icon-size 80 --text-size 12 --app-drop-link 370 225 --icon "Ricochet.app" 165 225 --volname "Ricochet" --hide-extension "Ricochet.app" --background "../icons/osx-dmg-background.tiff" "Ricochet.dmg" "osx-dmg-source/"

echo "---------"

otool -L Ricochet.app/Contents/MacOS/ricochet
otool -L Ricochet.app/Contents/MacOS/tor

codesign -vvvv -d Ricochet.app
spctl -vvvv --assess --type execute Ricochet.app

echo
echo "Output: ./build/Ricochet.dmg"

