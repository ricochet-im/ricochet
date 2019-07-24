Ricochet Refresh Build Scripts
----------------------

These are the scripts and instructions used to build distribution binaries for Ricochet Refresh, for various platforms.

If you don't know what Ricochet Refresh is or you're trying to find pre-built binaries, you're probably more interested in the [main repository](https://github.com/blueprint-freespeech/ricochet-refresh) or [website](https://ricochet.im/).

If you want to build your own Ricochet Refresh for local use, there are general [build instructions](https://github.com/blueprint-freespeech/ricochet-refresh/blob/master/BUILDING.md). These scripts might still be useful to you, because they take care of all dependencies and create redistributable binaries.

Platforms
---------

*linux-static* creates a portable Linux binary that statically links all non-standard libraries. It's intended to be a best-effort at creating one binary that can run unmodified on most modern Linux systems. The scripts are tailored to build from a Debian stable machine, but the resulting binaries should be compatible with many distributions.

*mingw-cross* cross-compiles 32bit Windows binaries using the MinGW-w64 toolchain. It's compatible with mingw packages included in many distributions. This is the recommended way to build for Windows machines.

*mingw* builds from a MSYS2/MinGW-w64 environment on Windows. This can be difficult to get working correctly, so cross-compilation is often a better choice.

*osx* builds a MacOS self-contained .app and packages it into .dmg. This requires MacOS with xcode and commandline tools.

Building
--------

See individual directories for build instructions. Generally, set `MAKEOPTS=-j6`, run `./platform/build-deps.sh`, checkout the commit you want to `src/ricochet`, and run `./platform/build.sh`

If a submodule has changed versions since the last time you ran a build, you will need to run `git submodule foreach --recursive git clean -x -f -d` to get submodules to a consistent state.

In the case of macOS, you can specify these flags to just use the brew versions of each library (since it's impossible to build qt5 on macOS Mojave) `USE_LOCAL_QT=1 USE_LOCAL_OPENSSL=1 USE_LOCAL_TOR=1 USE_LOCAL_PROTOBUF=1 osx/make-deps.sh`
