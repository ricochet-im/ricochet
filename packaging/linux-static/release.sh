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

VERSION=`git describe --tags HEAD`

. .packagingrc

if [ -z "$TOR_BINARY" ] || [ ! -f "$TOR_BINARY" ]; then
    echo "Missing TOR_BINARY: $TOR_BINARY"
    exit 1
fi

rm -r build || true
mkdir build
cd build

qmake CONFIG+=release ${QMAKEOPTS} ..
make

mkdir -p staging/torsion
# Copy binaries to staging area
cp Torsion staging/torsion/
cp "$TOR_BINARY" staging/torsion/
# Copy extra files
cp -r ../packaging/linux-static/content/* staging/torsion/

cd staging
tar cfj torsion-${VERSION}-static.tar.bz2 torsion
mv *.bz2 ..
cd ..

echo "---------"

tar fjt *.bz2

echo
echo "Output: ./build/torsion-${VERSION}-static.tar.bz2"

