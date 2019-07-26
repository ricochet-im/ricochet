#!/bin/bash

set -e

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
    git submodule update --init qtbase qtdeclarative qtimageformats qtquickcontrols qttools qttranslations qtwinextras qtmultimedia
    git submodule foreach git clean -dfx .
    git submodule foreach git reset --hard
    pushd qtbase
      git apply --ignore-whitespace "${ROOT_SRC}/../mingw/0001-Workarounds-for-a-dynamic-opengl-build-under-msys.patch"
      git apply --ignore-whitespace "${ROOT_SRC}/../mingw/0001-fix-visibility-of-bundled-zlib-symbols-with-mingw.patch"
    popd
    # Prefix needs to use the Windows style, not the mingw converted path
    QT_PREFIX="$(cygpath -m "${ROOT_LIB}/qt5/")"
    ./configure -prefix "${QT_PREFIX}" -release -opensource -confirm-license -no-dbus -no-qml-debug -no-glib -no-openssl -no-fontconfig -no-icu -qt-pcre -qt-zlib -qt-libpng -qt-libjpeg -nomake tools -nomake examples -platform win32-g++ -opengl dynamic
    make ${MAKEOPTS}
    make install
  popd

  # Openssl
  pushd openssl
    # CRLF can break a perl script used by openssl's build; reset core.autocrlf on this repo
    if [ "$(git config core.autocrlf)" != "false" ]; then
      echo "Fixing core.autocrlf on OpenSSL repository"
      git config core.autocrlf false
      git rm --cached -r .
    fi
    git clean -dfx .
    git reset --hard
    ./config no-shared no-zlib --prefix="${ROOT_LIB}/openssl/" --openssldir="${ROOT_LIB}/openssl/"
    make -j1
    make install
  popd

  # Libevent
  pushd libevent
    git clean -dfx .
    git reset --hard
    ./autogen.sh
    ./configure --prefix="${ROOT_LIB}/libevent" --disable-openssl
    make ${MAKEOPTS}
    make install
  popd

  # Tor
  pushd tor
    git clean -dfx .
    git reset --hard
    ./autogen.sh
    LIBS+=-lcrypt32 ./configure --prefix="${ROOT_LIB}/tor" --with-openssl-dir="${ROOT_LIB}/openssl/" --with-libevent-dir="${ROOT_LIB}/libevent/" --with-zlib-dir="$(pkg-config --variable=libdir zlib)" --enable-static-tor --disable-asciidoc
    make ${MAKEOPTS}
    make install
    cp "${ROOT_LIB}/tor/bin/tor.exe" "${BUILD_OUTPUT}/"
  popd

  # Protobuf
  pushd protobuf
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
    ./configure --prefix="${ROOT_LIB}/protobuf/" --disable-shared --without-zlib
    make ${MAKEOPTS}
    make install
  popd

popd
echo "build-deps: done"
