#!/bin/bash

set -e

ROOT_LIB=$(pwd)/lib
OPENSSLDIR="${OPENSSLDIR:-${ROOT_LIB}/openssl/}"

pushd "../tests"
  export PKG_CONFIG_PATH=${ROOT_LIB}/protobuf/lib/pkgconfig:${PKG_CONFIG_PATH}
  export PATH="${ROOT_LIB}/protobuf/bin/:${ROOT_LIB}/qt5/bin:${PATH}"

  qmake tests.pro CONFIG+=x86_64 CONFIG+=qtquickcompiler "OPENSSLDIR=$OPENSSLDIR" && make qmake_all && make
  TESTS=$(find . -type f -regextype sed -regex "./.*\(test_\|tst_\)[^/]*" -executable)

  if [ -n "$HEADLESS" ]; then
    Xvfb :1 -screen 0 800x600x16 &
    echo "$TESTS" | while read -r test; do xvfb-run -e /dev/stderr -a "$test" || exit $?; done
  else
    echo "$TESTS" | while read -r test; do "$test" || exit $?; done
  fi
popd

