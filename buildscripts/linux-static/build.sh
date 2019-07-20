#!/bin/bash

set -e

ROOT_LIB=$(pwd)/lib
BUILD_OUTPUT=$(pwd)/output

pushd ..
  RICOCHET_VERSION=$(git describe --tags HEAD)

  test -e build && rm -r build
  mkdir build
  pushd build

    export PKG_CONFIG_PATH=${ROOT_LIB}/protobuf/lib/pkgconfig:${PKG_CONFIG_PATH}
    export PATH=${ROOT_LIB}/qt5/bin/:${ROOT_LIB}/protobuf/bin/:${PATH}
    qmake CONFIG+=release OPENSSLDIR="${ROOT_LIB}/openssl/" ..
    make ${MAKEOPTS}
    cp ricochet-refresh "${BUILD_OUTPUT}/ricochet-refresh-unstripped"
    strip ricochet-refresh

    mkdir -p staging/ricochet-refresh
    cp ricochet-refresh staging/ricochet-refresh
    cp "${BUILD_OUTPUT}/tor" staging/ricochet-refresh
    cp -r ../packaging/linux-static/content/* staging/ricochet-refresh/

    pushd staging
      tar cfj "${BUILD_OUTPUT}/ricochet-refresh-${RICOCHET_VERSION}-static.tar.bz2" ricochet-refresh
    popd
  popd
popd

echo "---------------------"
ls -la "${BUILD_OUTPUT}/"
echo "build: done"
