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

export PKG_CONFIG_PATH=${ROOT_LIB}/protobuf/lib/pkgconfig:${PKG_CONFIG_PATH}
export PATH=${ROOT_LIB}/qt5/bin/:${ROOT_LIB}/protobuf/bin/:${PATH}
qmake CONFIG+=release OPENSSLDIR="${ROOT_LIB}/openssl/" ..
make "${MAKEOPTS}"
cp ricochet "${BUILD_OUTPUT}/ricochet-unstripped"
strip ricochet

mkdir -p staging/ricochet
cp ricochet staging/ricochet
cp "${BUILD_OUTPUT}/tor" staging/ricochet
# XXX
cp -r ../packaging/linux-static/content/* staging/ricochet/

cd staging
tar cfj "${BUILD_OUTPUT}/ricochet-${RICOCHET_VERSION}-static.tar.bz2" ricochet
cd ../../..

echo "---------------------"
ls -la "${BUILD_OUTPUT}/"
echo "build: done"
