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
cd "$ROOT_SRC"

# Qt
cd qt5
git submodule update --init qtbase qtdeclarative qtgraphicaleffects qtimageformats qtquickcontrols qtsvg qtx11extras qttools qtmultimedia
git submodule foreach git clean -dfx .
git submodule foreach git reset --hard
./configure -opensource -confirm-license -static -no-qml-debug -qt-zlib -qt-libpng -qt-libjpeg -qt-freetype -no-openssl -qt-pcre -qt-xcb -qt-xkbcommon -nomake tests -nomake examples -no-cups -prefix "${ROOT_LIB}/qt5/"
make "${MAKEOPTS}"
make install
cd ..

# Qt Declarative 2D Renderer
cd qtdeclarative-render2d
git clean -dfx .
git reset --hard
"${ROOT_LIB}/qt5/bin/qmake"
make "${MAKEOPTS}"
make install
cd ..

# Openssl
cd openssl
git clean -dfx .
git reset --hard
./config no-shared no-zlib no-dso --prefix="${ROOT_LIB}/openssl/" -fPIC
make -j1
make install
cd ..

# Libevent
cd libevent
git clean -dfx .
git reset --hard
./autogen.sh
./configure --prefix="${ROOT_LIB}/libevent" --disable-openssl
make "${MAKEOPTS}"
make install
cd ..

# Tor
cd tor
git clean -dfx .
git reset --hard
./autogen.sh
CFLAGS=-fPIC ./configure --prefix="${ROOT_LIB}/tor" --with-openssl-dir="${ROOT_LIB}/openssl/" --with-libevent-dir="${ROOT_LIB}/libevent/" --with-zlib-dir="$(pkg-config --variable=libdir zlib)" --enable-static-tor --disable-asciidoc
make "${MAKEOPTS}"
make install
cp "${ROOT_LIB}/tor/bin/tor" "${BUILD_OUTPUT}/"
cd ..

# Protobuf
cd protobuf
git clean -dfx .
git reset --hard

# Protobuf will rudely fetch this over HTTP if it isn't present..
if [ ! -e gtest ]; then
	git clone https://github.com/google/googletest.git gtest
	cd gtest
	git checkout release-1.5.0
	cd ..
fi

./autogen.sh
./configure --prefix="${ROOT_LIB}/protobuf/" --disable-shared --without-zlib --with-pic
make "${MAKEOPTS}"
make install
cd ..

cd ..
echo "build-deps: done"
