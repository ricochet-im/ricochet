## New Building

### Linux

Clone

Download and install the prebuilt Qt 5.15 ( https://www.qt.io/download-qt-installer ) and add bin directory containing 'qmake' to your PATH environment variable.

#### Debian

Install the following development packages:

```sh
apt-get install build-essential libssl-dev pkg-config libprotobuf-dev protobuf-compiler libgl-dev
```

## Building Ricochet Refresh

These instructions are intended for people who wish to build or modify Ricochet Refresh from source. Most users should [download releases](https://github.com/blueprint-freespeech/ricochet-refresh/releases) instead.

Clone with git from `https://github.com/blueprint-freespeech/ricochet-refresh.git`, or download source packages [on github](https://github.com/blueprint-freespeech/ricochet-refresh/releases). Then proceed to instructions for your platform.

If you're interested in helping to package Ricochet Refresh for common Linux platforms, please get in touch!

## Debug vs Release build

Add `CONFIG+=debug` or `CONFIG+=release` to the qmake command for a debug or release build. Debug builds enable logging to standard output, and shouldn't be used in sensitive environments.

## Linux

You will need:
 * Qt >= 5.1.0
 * OpenSSL (libcrypto)
 * pkg-config
 * Protocol Buffers (libprotobuf, protoc)

#### Fedora
```sh
yum install make gcc-c++ protobuf-devel protobuf-compiler openssl-devel
yum install qt5-qtbase qt5-qttools-devel qt5-qttools qt5-qtquickcontrols qt5-qtdeclarative qt5-qtbase-devel qt5-qtbase-gui qt5-qtdeclarative-devel qt5-qtmultimedia-devel
yum install tor # or build your own
```
#### Debian & Ubuntu
```sh
apt-get install build-essential libssl-dev pkg-config libprotobuf-dev protobuf-compiler
apt-get install qt5-qmake qt5-default qtbase5-dev qttools5-dev-tools qtdeclarative5-dev qtmultimedia5-dev
apt-get install qml-module-qtquick-controls qml-module-qtquick-dialogs qml-module-qtmultimedia
apt-get install tor # or build your own
```

If the `qml-module-qtquick` packages aren't available, try `qtdeclarative5-controls-plugin` instead.

#### Qt SDK
The [Qt SDK](https://www.qt.io/download/) is available for most Linux systems and includes an IDE as well as all Qt dependencies.

### Building and Installing
To build, simply run:
```sh
qmake # qmake-qt5 for some platforms
make
```

For a system-wide installation, use:
```sh
make install # as root
```

You must have a `tor` binary installed on the system (in `$PATH`), or placed next to the `ricochet-refresh` binary.

By default Ricochet Refresh will load and save configuration files to `~/.local/share/ricochet-refresh/`. First argument to command-line overrides this and allows you to specificy the config directory.

The [buildscripts](https://github.com/blueprint-freespeech/ricochet-refresh/tree/master/buildscripts) directory contains a set of scripts to build a fully static Ricochet Refresh on a clean Debian system. These are used to create the generic linux binary packages.

#### Hardening
Ricochet Refresh will use aggressive compiler hardening flags if available. `qmake` will print the results of these tests on first run, or when run with `CONFIG+=recheck`.

To take full advantage of the sanitizer options, you may need to install `libasan` and `libubsan`.

## OS X

You will need:
 * Xcode (for toolchain)
 * Qt 5 - preferably the [Qt SDK](https://www.qt.io/download/)
 * Protocol Buffers (libprotobuf, protoc) - `brew install protobuf`
 * pkg-config - `brew install pkg-config`

You can either load `ricochet.pro` in Qt Creator and build normally, or build command-line with:
```sh
/path/to/qtsdk/5.3/clang_64/bin/qmake
make
```

You also need a `tor` binary in $PATH or inside the build's `ricochet refresh.app/Contents/MacOS` folder. The easiest solution is to use `brew install tor`. If you copy the `tor` binary, you will need to keep it up to date.

By default, configuration will be stored at `~/Library/Application Support/ricochet-refresh/` folder. This can be overriden by supplying a directory as first argument to the command-line.

The `packaging/osx/release_osx.sh` script demonstrates how to build a redistributable app bundle.

Since the openssl header files were removed in El Capitan, have qmake use the openssl that comes with brew (see the OPENSSLDIR var below).

Steps:
```
brew install protobuf qt5 tor
/usr/local/opt/qt5/bin/qmake OPENSSLDIR=/usr/local/opt/openssl/ CONFIG+=debug
make
```

## Windows

Building for Windows is difficult. The process and scripts used for release builds are documented in the [buildscripts directory](https://github.com/blueprint-freespeech/ricochet-refresh/tree/master/buildscripts).

For development builds, you will want:
 * Visual Studio C++ or MinGW
 * Qt 5 - preferably the [Qt SDK](https://www.qt.io/download/)
 * OpenSSL (including libs and headers)
 * Protocol Buffers >= 2.6.0

Compile OpenSSL and protobuf first, according to their instructions.

After installing the Qt SDK, open the `ricochet.pro` project in Qt Creator. Before building, you must click the 'Projects' tab on the left side, and under 'Build Steps', modify 'Additional arguments' to add:

```
    OPENSSLDIR=C:\path\to\openssl\ PROTOBUFDIR=C:\path\to\protobuf
```

Use the 'Build -> Run qmake' menu to test your changes.

You also need a `tor.exe` binary, placed in the same folder as `ricochet.exe`.

The windows installer can be built using Inno Setup. See `packaging\installer` for more information.
