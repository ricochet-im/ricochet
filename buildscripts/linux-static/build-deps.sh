#!/bin/bash

set -ex

ROOT_SRC="$(pwd)/src"
ROOT_LIB="$(pwd)/lib"
BUILD_OUTPUT="$(pwd)/output"
MAKEOPTS="-j$(nproc)"
test -e "${ROOT_SRC}"
test -e "${ROOT_LIB}" && rm -r "${ROOT_LIB}"
mkdir "${ROOT_LIB}"
test -e "${BUILD_OUTPUT}" && rm -r "${BUILD_OUTPUT}"
mkdir "${BUILD_OUTPUT}"

# Skip build deps if we're in travis
if [ -n "$HAS_JOSH_K_SEAL_OF_APPROVAL" ]; then
  curl https://github.com/blueprint-freespeech/ricochet-refresh/releases/download/1.1.4e/linux-static-v1.1.4-153-ge6bb36f.zip -o lib.zip
  unzip "lib.zip"
  exit 0
fi

# Build dependencies
git submodule update --init
pushd "$ROOT_SRC"

  # Qt
  pushd qt5
    if [[ -z $USE_LOCAL_QT ]]; then
      git submodule deinit --all -f
      git submodule update --init qtbase qtdeclarative qtgraphicaleffects qtimageformats qtquickcontrols qtsvg qtx11extras qttools qtmultimedia
      git submodule foreach --recursive git clean -dfx .
      git submodule foreach --recursive git reset --hard
      git clean -dfx .
      git reset --hard
      ./configure \
        -confirm-license \
        -no-compile-examples \
        -no-cups \
        -no-openssl \
        -no-qml-debug \
        -nomake examples \
        -nomake tests \
        -opensource \
        -prefix "${ROOT_LIB}/qt5/" \
        -qt-freetype \
        -qt-libjpeg \
        -qt-libpng \
        -qt-pcre \
        -qt-xcb \
        -qt-zlib \
        -release \
        -static \
        -xkbcommon
     make ${MAKEOPTS}
      make install
      if [ ! -f "${ROOT_LIB}/qt5/bin/qmake" ]; then
        ln -s "$(pwd)/qtbase/bin/qmake" "${ROOT_LIB}/qt5/bin/qmake"
      fi
    fi
    export PATH="${ROOT_LIB}/qt5/bin:${PATH}"
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
