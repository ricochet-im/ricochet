#!/bin/bash

set -e

ROOT_LIB=$(pwd)/lib
OPENSSLDIR="${OPENSSLDIR:-${ROOT_LIB}/openssl/}"

pushd "../tests"
  export PKG_CONFIG_PATH=${ROOT_LIB}/protobuf/lib/pkgconfig:${PKG_CONFIG_PATH}
  export PATH=${ROOT_LIB}/protobuf/bin/:${PATH}

  qmake tests.pro -spec macx-clang CONFIG+=x86_64 CONFIG+=qtquickcompiler OPENSSLDIR="$OPENSSLDIR" && /usr/bin/make qmake_all
  make ${MAKEOPTS}

  /usr/bin/find -E . -type f -regex "./.*(test_|tst_)[^/]*" -perm +111 | while read -r test; do $test; echo "test $test exited with $?"; done
popd
