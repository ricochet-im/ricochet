ROOT_SRC=$(pwd)/src
ROOT_LIB=$(pwd)/lib

pushd "$ROOT_SRC/ricochet-refresh/tests"
  export PKG_CONFIG_PATH=${ROOT_LIB}/protobuf/lib/pkgconfig:${PKG_CONFIG_PATH}
  export PATH=${ROOT_LIB}/protobuf/bin/:${PATH}

  qmake tests.pro -spec macx-clang CONFIG+=x86_64 CONFIG+=qtquickcompiler OPENSSLDIR="${ROOT_LIB}/openssl/" && /usr/bin/make qmake_all
  make ${MAKEOPTS}

  # GNU find
  # find . -type f -regextype sed -regex "./.*\(test_\|tst_\)[^/]*" -executable -exec {} \;
  # BSD Find
  /usr/bin/find -E . -type f -regex "./.*(test_|tst_)[^/]*" -perm +111 -exec {} \;
popd
