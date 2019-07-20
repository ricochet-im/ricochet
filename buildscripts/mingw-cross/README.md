These scripts will cross-compile Ricochet for Windows from a unixy system with the MinGW toolchain available.

Prerequisites
-------------

Install the mingw-w64 32bit toolchain, which is often available through package management.

Fedora (as of 24) needs the following packages:

```
python
m4 autoconf automake flex bison libtool
mingw32-filesystem mingw32-crt mingw32-headers mingw32-bintuils mingw32-cpp mingw32-gcc mingw32-gcc-c++
mingw32-pkg-config mingw32-zlib-static mingw32-winpthreads-static
```

The `0001-windeployqt-Hack-to-use-objdump-for-PE-parsing.patch` patch assumes a 32bit release build, and assumes that compiler library DLLs are in `/usr/i686-w64-mingw32/sys-root/mingw/bin/`. If these aren't true, you should modify the patch. A better solution is needed here.

Build dependencies
------------------

All other dependencies are included as submodules of this repository. The scripts will handle init/update of the submodules. Dependencies are installed to `lib` under this repository. Nothing is installed or changed outside of this repository directory.

Build dependencies with:

```
export MAKEOPTS=-j6 # Or whatever -j value you prefer
./mingw-cross/build-deps.sh
```

Build Ricochet
--------------

Clone ricochet under the `src/` directory. If you want to build something other than `master`, checkout the branch/commit of your choice.

```
cd src
git clone https://github.com/ricochet-im/ricochet
```

The build script will not change HEAD or discard working copy changes in the ricochet repository, **but** it will be fully `git clean`ed (including untracked files) for every build.

Build packages with:

```
./mingw-cross/build.sh
```

If successful, the `output` directory will contain `ricochet-VERSION.zip` and `ricochet-VERSION-installer-build.zip` as well as `tor.exe` and `ricochet.exe`. The zip files contain all necessary libraries and files. The `-installer-build.zip` file is not suitable to use directly, but contains everything necessary to build the installer with a copy of Inno Setup.

