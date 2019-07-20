#!/bin/bash

set -e

# Ensure PATH is set correctly
which iscc >/dev/null

ROOT_SRC=$(pwd)/src
ROOT_LIB=$(pwd)/lib
BUILD_OUTPUT=$(pwd)/output

cd "$ROOT_SRC"

# Ricochet
test -e ricochet || git clone https://github.com/ricochet-im/ricochet.git
cd ricochet
git clean -dfx .

test -e build && rm -r build
mkdir build
cd build

export PATH=${ROOT_LIB}/qt5/bin/:${ROOT_LIB}/protobuf/bin/:${PATH}
qmake CONFIG+=release OPENSSLDIR="${ROOT_LIB}/openssl/" PROTOBUFDIR="${ROOT_LIB}/protobuf/" ..
make "${MAKEOPTS}"
cp release/ricochet.exe "${BUILD_OUTPUT}/"

mkdir installer
cd installer
cp "${BUILD_OUTPUT}/ricochet.exe" .
cp "${BUILD_OUTPUT}/tor.exe" .
"${ROOT_LIB}/qt5/bin/windeployqt" --qmldir ../../src/ui/qml --dir Qt ricochet.exe
test -e Qt/qmltooling && rm -r Qt/qmltooling
# libssp is needed for gcc's stack protector, and windeployqt won't grab it
cp "$(which libssp-0.dll)" Qt/
cp ../../packaging/installer/* .
iscc installer.iss
cp Output/Ricochet.exe "${BUILD_OUTPUT}/setup.exe"
cd ../../..

echo "---------------------"
ls -la "${BUILD_OUTPUT}/"
echo "build: done"
