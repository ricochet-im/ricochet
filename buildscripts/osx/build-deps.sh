#!/bin/bash

set -ex

ROOT_SRC=$(pwd)/src
ROOT_LIB=$(pwd)/lib
BUILD_OUTPUT=$(pwd)/output
test -e "${ROOT_SRC}"
test -e "${ROOT_LIB}" && rm -rf "${ROOT_LIB}"
mkdir "${ROOT_LIB}"
test -e "${BUILD_OUTPUT}" && rm -rf "${BUILD_OUTPUT}"
mkdir "${BUILD_OUTPUT}"
MACOS_VERSION_MIN="${MACOS_VERSION_MIN:-10.7}"

if [[ -n $USE_LOCAL_TOR ]]; then
  echo "Warning: Using local tor means libevent is not statically compiled into the tor binary, making it less portable."
  sleep 5
fi

# Build dependencies
git submodule update --init
pushd "$ROOT_SRC"

  # Qt
  pushd qt5
    if [[ -n $USE_LOCAL_QT ]]; then
      if ! brew info qt5 > /dev/null; then
        echo "please install qt5 using brew"
        exit 1
      fi
      ln -s "$(brew --prefix qt5)" "${ROOT_LIB}/qt5"
    else
      git submodule update --init qtbase qtdeclarative qtgraphicaleffects qtimageformats qtquickcontrols qtsvg qtmacextras qttools qtmultimedia
      git submodule foreach git clean -dfx .
      git submodule foreach git reset --hard
      ./configure -opensource -confirm-license -release \
          -no-qml-debug -no-dbus -no-openssl -no-cups \
          -qt-zlib -qt-libpng -qt-libjpeg -qt-freetype -qt-pcre \
          -nomake tests -nomake examples \
          -prefix "${ROOT_LIB}/qt5/"
      make ${MAKEOPTS}
      make install
    fi
    export PATH="$PATH:${ROOT_LIB}/qt5/bin"
  popd

  if ! command -v qmake; then
    echo "qmake not found"
    exit 1
  fi

  # Openssl
  pushd openssl
    if [[ -n $USE_LOCAL_OPENSSL ]]; then
      if ! brew info openssl > /dev/null; then
        echo "please install openssl using brew"
        exit 1
      fi
      ln -s "$(brew --prefix openssl)" "${ROOT_LIB}/openssl"
    else
      git clean -dfx .
      git reset --hard
      ./Configure no-shared no-zlib "--prefix=${ROOT_LIB}/openssl/" \
        "--openssldir=${ROOT_LIB}/openssl/" -fPIC "-mmacosx-version-min=${MACOS_VERSION_MIN}" \
        darwin64-x86_64-cc
      make -j1
      make install
    fi
  popd

  # Libevent
  pushd libevent
    if [[ -n $USE_LOCAL_LIBEVENT ]]; then
      if ! brew info libevent > /dev/null; then
        echo "please install libevent using brew"
        exit 1
      fi
      ln -s "$(brew --prefix libevent)" "${ROOT_LIB}/libevent"
    else
      git clean -dfx .
      git reset --hard
      # git apply "${ROOT_SRC}/../osx/libevent-0001-Forcefully-disable-clock_gettime-on-macOS-due-to-a-S.patch"
      ./autogen.sh
      CFLAGS="-mmacosx-version-min=${MACOS_VERSION_MIN}" ./configure "--prefix=${ROOT_LIB}/libevent" --disable-openssl
      make ${MAKEOPTS}
      make install
    fi
  popd

  # Tor
  pushd tor
    if [[ -n $USE_LOCAL_TOR ]]; then
      if ! brew info tor > /dev/null; then
        echo "please install tor using brew"
        exit 1
      fi
      ln -s "$(brew --prefix tor)" "${ROOT_LIB}/tor"
    else
      git clean -dfx .
      git reset --hard
      # git apply "${ROOT_SRC}/../osx/tor-0001-Forcefully-disable-getentropy-and-clock_gettime-on-m.patch"
      ./autogen.sh
      CFLAGS="-fPIC -mmacosx-version-min=${MACOS_VERSION_MIN}" ./configure --prefix="${ROOT_LIB}/tor" \
        --with-openssl-dir="${ROOT_LIB}/openssl/" --enable-static-openssl \
        --with-libevent-dir="${ROOT_LIB}/libevent/" --enable-static-libevent \
        --disable-asciidoc --disable-libscrypt --disable-lzma
      make ${MAKEOPTS}
      make install
    fi
    cp "${ROOT_LIB}/tor/bin/tor" "${BUILD_OUTPUT}/"
    chmod +w "${BUILD_OUTPUT}/tor"
  popd

  # Protobuf
  pushd protobuf
    if [[ -n $USE_LOCAL_PROTOBUF ]]; then
      if ! brew info protobuf > /dev/null; then
        echo "please install protobuf using brew"
        exit 1
      fi
      ln -s "$(brew --prefix protobuf)" "${ROOT_LIB}/protobuf"
    else
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
      CXX=clang++ CXXFLAGS="-mmacosx-version-min=${MACOS_VERSION_MIN} -stdlib=libc++" ./configure \
        "--prefix=${ROOT_LIB}/protobuf/" --disable-shared --without-zlib --with-pic
      make ${MAKEOPTS}
      make install
    fi
  popd

popd
echo "build-deps: done"
