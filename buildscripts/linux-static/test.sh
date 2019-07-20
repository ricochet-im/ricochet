#!/bin/bash

set -e

ROOT_LIB=$(pwd)/lib

pushd "../tests"
  export PKG_CONFIG_PATH=${ROOT_LIB}/protobuf/lib/pkgconfig:${PKG_CONFIG_PATH}
  export PATH=${ROOT_LIB}/protobuf/bin/:${PATH}

  qmake tests.pro CONFIG+=x86_64 CONFIG+=qtquickcompiler OPENSSLDIR="${ROOT_LIB}/openssl/" && make qmake_all
  make ${MAKEOPTS}

  find . -type f -regextype sed -regex "./.*\(test_\|tst_\)[^/]*" -executable | while read -r test; do $test; done
popd
