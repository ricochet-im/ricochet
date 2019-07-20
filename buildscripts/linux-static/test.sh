#!/bin/bash

set -e

ROOT_LIB=$(pwd)/lib
OPENSSLDIR="${OPENSSLDIR:-${ROOT_LIB}/openssl/}"

pushd "../tests"
  export PKG_CONFIG_PATH=${ROOT_LIB}/protobuf/lib/pkgconfig:${PKG_CONFIG_PATH}
  export PATH=${ROOT_LIB}/protobuf/bin/:${PATH}

  MAKE_COMMAND=$'qmake tests.pro CONFIG+=x86_64 CONFIG+=qtquickcompiler OPENSSLDIR=$OPENSSLDIR && make qmake_all && make'
  TEST_COMMAND=$'find . -type f -regextype sed -regex "./.*\(test_\|tst_\)[^/]*" -executable | while read -r test; do $test || exit $?; done'

  if [ -n "$HEADLESS" ]; then
    docker pull garthk/qt-build:bionic-5.12.0
    docker run -d -v $PWD/..:/repo -w /repo/tests --name tester garthk/qt-build:bionic-5.12.0 tail -f /dev/null
    docker exec -it tester bash -c "apt-get update && apt-get -yq --no-install-suggests --no-install-recommends install libssl-dev libprotobuf-dev protobuf-compiler qt5-qmake qt5-default qtbase5-dev qttools5-dev-tools qtdeclarative5-dev pkg-config"
    docker exec -it --env "PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig" tester bash -c "$MAKE_COMMAND"
    docker exec -it tester bash -c "$TEST_COMMAND"
  else
    "$MAKE_COMMAND"
    "$TEST_COMMAND"
  fi
popd

