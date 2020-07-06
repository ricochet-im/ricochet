#!/bin/bash

set -ex

ROOT_SRC="$(pwd)/src"
ROOT_LIB="$(pwd)/lib"
BUILD_OUTPUT="$(pwd)/output"
# MAKEOPTS="-j$(nproc)"
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
      git submodule deinit --all -f
      git submodule update --init qtbase qtdeclarative qtgraphicaleffects qtimageformats qtquickcontrols qtsvg qtx11extras qttools qtmultimedia
      git submodule foreach --recursive git reset --hard
      git reset --hard
      # TODO:
      # Find which configure options can be included to compile QT without breaking the compilation

      # Missing configure options
      # -no-feature-bearermanagement \
      # -no-feature-big_codecs \
      # -no-feature-calendarwidget \
      # -no-feature-codecs \
      # -no-feature-colordialog \
      # -no-feature-colornames \
      # -no-feature-concurrent \
      # -no-feature-completer \
      # -no-feature-cups \
      # -no-feature-datawidgetmapper \
      # -no-feature-dbus \
      # -no-feature-desktopservices \
      # -no-feature-dom \
      # -no-feature-effects \
      # -no-feature-errormessage \
      # -no-feature-filedialog \
      # -no-feature-filesystemmodel \
      # -no-feature-filesystemwatcher \
      # -no-feature-fontdialog \
      # -no-feature-freetype \
      # -no-feature-fscompleter \
      # -no-feature-ftp \
      # -no-feature-gestures \
      # -no-feature-graphicseffect \
      # -no-feature-graphicsview \
      # -no-feature-iconv \
      # -no-feature-im \
      # -no-feature-image_heuristic_mask \
      # -no-feature-image_text \
      # -no-feature-imageformat_bmp \
      # -no-feature-imageformat_jpeg \
      # -no-feature-imageformat_png \
      # -no-feature-imageformat_ppm \
      # -no-feature-imageformat_xbm \
      # -no-feature-inputdialog \
      # -no-feature-keysequenceedit \
      # -no-feature-mimetype \
      # -no-feature-networkdiskcache \
      # -no-feature-networkproxy \
      # -no-feature-paint_debug \
      # -no-feature-printpreviewdialog \
      # -no-feature-printpreviewwidget \
      # -no-feature-process \
      # -no-feature-progressdialog \
      # -no-feature-sharedmemory \
      # -no-feature-sizegrip \
      # -no-feature-socks5 \
      # -no-feature-statemachine \
      # -no-feature-systemsemaphore \
      # -no-feature-systemtrayicon \
      # -no-feature-texthtmlparser \
      # -no-feature-textodfwriter \
      # -no-feature-translation \
      # -no-feature-udpsocket \
      # -no-feature-undocommand \
      # -no-feature-undogroup \
      # -no-feature-undostack \
      # -no-feature-undoview \
      # -no-feature-wizard \
      # -skip qtdeclarative \
      # -skip qtimageformats \
      # -skip qtmultimedia \
      # -skip qtquickcontrols \
      # -skip qtquickcontrols2 \
      # -skip qtsvg \
      # -skip qttools \
      # -skip qttranslations \
      # -skip qtx11extras \
      #
      ./configure \
        -confirm-license \
        -fontconfig \
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
        -skip qt3d \
        -skip qtandroidextras \
        -skip qtcanvas3d \
        -skip qtcharts \
        -skip qtconnectivity \
        -skip qtdatavis3d \
        -skip qtdoc \
        -skip qtgamepad \
        -skip qtgraphicaleffects \
        -skip qtlocation \
        -skip qtmacextras \
        -skip qtnetworkauth \
        -skip qtpurchasing \
        -skip qtscript \
        -skip qtscxml \
        -skip qtsensors \
        -skip qtserialbus \
        -skip qtserialport \
        -skip qtspeech \
        -skip qtvirtualkeyboard \
        -skip qtwayland \
        -skip qtwebchannel \
        -skip qtwebengine \
        -skip qtwebsockets \
        -skip qtwebview \
        -skip qtwinextras \
        -skip qtxmlpatterns \
        -static \
        -system-freetype \
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
