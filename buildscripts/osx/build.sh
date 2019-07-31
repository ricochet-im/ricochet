#!/bin/bash

set -ex
source common_functions.sh
ROOT_LIB=$(pwd)/lib
BUILD_OUTPUT=$(pwd)/output

OPENSSLDIR="${OPENSSLDIR:-${ROOT_LIB}/openssl/}"

test -e "${BUILD_OUTPUT}/Ricochet Refresh.app" && rm -rf "${BUILD_OUTPUT}/Ricochet Refresh.app"
test -e "${BUILD_OUTPUT}/ricochet-refresh-unstripped" && rm -rf "${BUILD_OUTPUT}/ricochet-refresh-unstripped"
test -e "${BUILD_OUTPUT}/Ricochet*.dmg" && rm -rf "${BUILD_OUTPUT}/Ricochet*.dmg"

pushd ..
  RICOCHET_VERSION=$(git_version)

  test -e build && rm -r build
  mkdir build

  pushd build

    export PKG_CONFIG_PATH=${ROOT_LIB}/protobuf/lib/pkgconfig:${PKG_CONFIG_PATH}
    export PATH="${ROOT_LIB}/protobuf/bin/:${ROOT_LIB}/qt5/bin:${PATH}"
    qmake CONFIG+=release OPENSSLDIR="$OPENSSLDIR" ..
    make ${MAKEOPTS}

    cp ricochet-refresh.app/Contents/MacOS/ricochet-refresh "${BUILD_OUTPUT}/ricochet-refresh-unstripped"
    cp "${BUILD_OUTPUT}/tor" "ricochet-refresh.app/Contents/MacOS"
    strip ricochet-refresh.app/Contents/MacOS/*

    mv ricochet-refresh.app "Ricochet Refresh.app"
    macdeployqt "Ricochet Refresh.app" -qmldir=../src/ui/qml
    cp -R "Ricochet Refresh.app" "${BUILD_OUTPUT}/"

    pushd "${BUILD_OUTPUT}"

      if [ -n "$CODESIGN_ID" ]; then
        codesign --verbose --sign "$CODESIGN_ID" --deep Ricochet\ Refresh.app
        # Sign twice to work around a bug(?) that results in the asan library being invalid
        codesign -f --verbose --sign "$CODESIGN_ID" --deep Ricochet\ Refresh.app
        codesign -vvvv -d Ricochet\ Refresh.app
      fi

      hdiutil create "Ricochet Refresh.dmg" -srcfolder "Ricochet Refresh.app" -format UDZO -volname "Ricochet Refresh"
      codesign -f --verbose --sign "$CODESIGN_ID" --deep Ricochet\ Refresh.dmg
      mv "Ricochet Refresh.dmg" "${BUILD_OUTPUT}/Ricochet-Refresh-${RICOCHET_VERSION}.dmg"
    popd
  popd

  echo "---------------------"
  ls -la "${BUILD_OUTPUT}/"
  if [ -z "$SKIP_CODESIGN" ]; then
    spctl -vvvv --assess --type execute "${BUILD_OUTPUT}/Ricochet Refresh.app"
  fi
  echo "build: done"
popd
