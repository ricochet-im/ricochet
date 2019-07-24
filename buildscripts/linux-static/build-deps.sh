#!/bin/bash

set -ex

ROOT_SRC=$(pwd)/src
ROOT_LIB=$(pwd)/lib
BUILD_OUTPUT=$(pwd)/output
test -e "${ROOT_SRC}"
test -e "${ROOT_LIB}" && rm -r "${ROOT_LIB}"
mkdir "${ROOT_LIB}"
test -e "${BUILD_OUTPUT}" && rm -r "${BUILD_OUTPUT}"
mkdir "${BUILD_OUTPUT}"

# Build dependencies
git submodule update --init
pushd "$ROOT_SRC"

  # Qt
  pushd qt5
    if [[ -z $USE_LOCAL_QT ]]; then
      git submodule update --init qtbase qtdeclarative qtgraphicaleffects qtimageformats qtquickcontrols qtsvg qtx11extras qttools qtmultimedia
      git submodule foreach git clean -dfx .
      git submodule foreach git reset --hard
      ./configure -opensource -confirm-license -static -no-qml-debug -qt-zlib \
        -qt-libpng -qt-libjpeg -qt-freetype -no-openssl -qt-pcre -qt-xcb \
        -nomake tests -nomake examples -no-cups -prefix "${ROOT_LIB}/qt5/"
      make ${MAKEOPTS}
      make install
    fi
  popd

  if ! command -v qmake; then
    echo "qmake not found"
    exit 1
  fi

  # Openssl
  pushd openssl
    if [[ -n $USE_LOCAL_OPENSSL ]]; then
      OPENSSL_DIR="$(pkg-config --variable=libdir openssl)"
    else
      OPENSSL_DIR="${ROOT_LIB}/openssl"
      git clean -dfx .
      git reset --hard
      ./config no-shared no-zlib no-dso "--prefix=${OPENSSL_DIR}" "--openssldir=${OPENSSL_DIR}" -fPIC
      make -j1
      make install_sw
    fi
  popd

  # Libevent
  pushd libevent
    if [[ -n $USE_LOCAL_LIBEVENT ]]; then
      LIBEVENT_DIR="$(pkg-config --variable=libdir libevent)"
    else
      LIBEVENT_DIR="${ROOT_LIB}/libevent"
      git clean -dfx .
      git reset --hard
      ./autogen.sh
      ./configure "--prefix=${LIBEVENT_DIR}" --disable-openssl
      make ${MAKEOPTS}
      make install
    fi
  popd

  # Tor
  pushd tor
    git clean -dfx .
    git reset --hard
    ./autogen.sh
    CFLAGS=-fPIC ./configure \
      --prefix="${ROOT_LIB}/tor" \
      --with-openssl-dir="${ROOT_LIB}/openssl/" \
      --with-libevent-dir="${ROOT_LIB}/libevent/" \
      --with-zlib-dir="$(pkg-config --variable=libdir zlib)" \
      --enable-static-tor --disable-asciidoc

    make ${MAKEOPTS}
    make install
    cp "${ROOT_LIB}/tor/bin/tor" "${BUILD_OUTPUT}/"
  popd

  # Protobuf
  pushd protobuf
    if [[ -z $USE_LOCAL_PROTOBUF ]]; then
      git clean -dfx .
      git reset --hard

      # Protobuf will rudely fetch this over HTTP if it isn't present..
      if [ ! -e gtest ]; then
        git clone https://github.com/google/googletest.git gtest
        pushd gtest
        git checkout release-1.5.0
        popd
      fi

      ./autogen.sh
      ./configure "--prefix=${ROOT_LIB}/protobuf/" --disable-shared --without-zlib --with-pic
      make ${MAKEOPTS}
      make install
    fi
  popd

popd
echo "build-deps: done"
