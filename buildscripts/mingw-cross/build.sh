#!/bin/bash

set -e

ROOT_SRC=$(pwd)/src
ROOT_LIB=$(pwd)/lib
BUILD_OUTPUT=$(pwd)/output

cd "$ROOT_SRC"

# Ricochet
test -e ricochet || git clone https://github.com/ricochet-im/ricochet.git
cd ricochet
git clean -dfx .

RICOCHET_VERSION=$(git describe --tags HEAD)

test -e build && rm -r build
mkdir build
cd build

export PATH=${ROOT_LIB}/qt5/bin/:${ROOT_LIB}/protobuf-native/bin/:${PATH}
qmake CONFIG+=release OPENSSLDIR="${ROOT_LIB}/openssl/" PROTOBUFDIR="${ROOT_LIB}/protobuf/" ..
make ${MAKEOPTS}
cp release/ricochet.exe "${BUILD_OUTPUT}/"

mkdir -p staging/ricochet
cd staging/ricochet
cp "${BUILD_OUTPUT}/ricochet.exe" .
cp "${BUILD_OUTPUT}/tor.exe" .
# windeployqt is (hackishly) patched to support reading PE binaries on non-windows
"${ROOT_LIB}/qt5/bin/windeployqt" --qmldir ../../../src/ui/qml ricochet.exe
test -e qmltooling && rm -r qmltooling
test -e imageformats && rm -r imageformats
test -e playlistformats && rm -r playlistformats
cd ..
zip -r "${BUILD_OUTPUT}/ricochet-${RICOCHET_VERSION}.zip" ricochet

mkdir installer
cd installer
cp "${BUILD_OUTPUT}/ricochet.exe" .
cp "${BUILD_OUTPUT}/tor.exe" .
cp ../../../packaging/installer/* ../../../icons/ricochet.ico ../../../LICENSE .
mkdir translation
cp -r ../../../translation/{inno,installer_*.isl} translation
"${ROOT_LIB}/qt5/bin/windeployqt" --qmldir ../../../src/ui/qml --dir Qt ricochet.exe
test -e Qt/qmltooling && rm -r Qt/qmltooling
test -e Qt/imageformats && rm -r Qt/imageformats
test -e Qt/playlistformats && rm -r Qt/playlistformats
cd ..
zip -r "${BUILD_OUTPUT}/ricochet-${RICOCHET_VERSION}-installer-build.zip" installer
cd ../../../..

echo "---------------------"
ls -la "${BUILD_OUTPUT}/"
echo "build: done"
