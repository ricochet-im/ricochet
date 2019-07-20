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
git submodule update --init qtbase qtdeclarative qtimageformats qtquickcontrols qttools qttranslations qtwinextras qtmultimedia
git submodule foreach git clean -dfx .
git submodule foreach git reset --hard
cd qttools
git apply --ignore-whitespace "${ROOT_SRC}/../mingw-cross/0001-windeployqt-Hack-to-use-objdump-for-PE-parsing.patch"
cd ..
./configure -prefix "${ROOT_LIB}/qt5/" -release -opensource -confirm-license -no-dbus -no-qml-debug -no-glib -no-openssl -no-fontconfig -no-icu -qt-pcre -qt-zlib -qt-libpng -qt-libjpeg -nomake tools -nomake examples -xplatform win32-g++ -device-option "CROSS_COMPILE=/usr/bin/i686-w64-mingw32-"
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
./Configure no-shared no-zlib no-capieng --prefix="${ROOT_LIB}/openssl/" --cross-compile-prefix=i686-w64-mingw32- mingw
make -j1 depend
make -j1
make install
cd ..

# Libevent
cd libevent
git clean -dfx .
git reset --hard
./autogen.sh
./configure --prefix="${ROOT_LIB}/libevent" --host=i686-w64-mingw32
make "${MAKEOPTS}"
make install
cd ..

# Tor
cd tor
git clean -dfx .
git reset --hard
./autogen.sh
./configure --prefix="${ROOT_LIB}/tor" --host=i686-w64-mingw32 --with-openssl-dir="${ROOT_LIB}/openssl/" --with-libevent-dir="${ROOT_LIB}/libevent/" --with-zlib-dir="$(i686-w64-mingw32-pkg-config --variable=libdir zlib)" --enable-static-tor --disable-asciidoc
make "${MAKEOPTS}"
make install
cp "${ROOT_LIB}/tor/bin/tor.exe" "${BUILD_OUTPUT}/"
cd ..

# Protobuf
cd protobuf
git clean -dfx .
git reset --hard
# Protobuf will rudely fetch this over HTTP if it isn't present..
if [ ! -e gtest ]; then
	git clone https://github.com/google/googletest.git gtest
	cd gtest
	git checkout release-1.7.0
	cd ..
fi
./autogen.sh
# Build native protobuf (for the protoc compiler)
./configure --prefix="${ROOT_LIB}/protobuf-native/" --disable-shared --without-zlib
make "${MAKEOPTS}"
make install
# Build protobuf for target
make distclean
./configure --prefix="${ROOT_LIB}/protobuf/" --host=i686-w64-mingw32 --with-protoc="${ROOT_LIB}/protobuf-native/bin/protoc" --disable-shared --without-zlib
make "${MAKEOPTS}"
make install
cd ..

cd ..
echo "build-deps: done"
