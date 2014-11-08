## Building Ricochet

These instructions are intended for people who wish to build or modify Ricochet from source. Most users should [download releases](https://github.com/ricochet-im/ricochet/releases) instead.

Clone with git from `https://github.com/ricochet-im/ricochet.git`, or download source packages [on github](https://github.com/ricochet-im/ricochet/releases). Then proceed to instructions for your platform.

## Hints

Add `CONFIG+=debug` or `CONFIG+=release` to the qmake command for a debug or release build. Debug builds enable logging to standard output, and shouldn't be used in sensitive environments.

By default, Ricochet will be portable, and configuration is stored in a folder named `config` next to the binary. Add `DEFINES+=RICOCHET_NO_PORTABLE` to the qmake command for a system-wide installation using platform configuration paths instead.

## Linux

You will need:
 * [Qt >= 5.1.0](https://qt-project.org/downloads)
 * [OpenSSL (libcrypto)](https://www.openssl.org/source/)
 * [Tor](https://www.torproject.org/download/download.html)

#### Fedora
```sh
yum install make gcc-c++ qt5-qtbase qt5-qttools-devel qt5-qttools qt5-qtquickcontrols qt5-qtdeclarative qt5-qtbase-devel qt5-qtbase-gui qt5-qtdeclarative-devel openssl-devel
yum install tor # or build your own
```
#### Ubuntu 14.04 or later
```sh
apt-get install build-essential libssl-dev pkg-config qt5-qmake qt5-default qtbase5-dev qttools5-dev-tools qtdeclarative5-dev qtdeclarative5-controls-plugin
apt-get install tor # or build your own
```
#### Arch Linux
Just use [ricochet](https://aur.archlinux.org/packages/ricochet/) package from AUR:
```sh
# download via yaourt or cower or via curl+tar(must be exist in any Arch system)
yaourt -G ricochet || cower -d ricochet || { curl "https://aur.archlinux.org/packages/ri/ricochet/ricochet.tar.gz" | tar -xzv; }  # or ricochet-git if you want lastest
# and build/install it
cd ricochet && makepkg -si
```

#### Qt SDK
The [Qt SDK](https://qt-project.org/downloads) is available for most Linux systems and includes an IDE as well as all Qt dependencies.

To build, simply run:
```sh
qmake # qmake-qt5 for some platforms
make
```

For a system-wide installation, use:
```sh
qmake DEFINES+=RICOCHET_NO_PORTABLE
make
make install # as root
```

You must have a `tor` binary installed on the system (in $PATH), or placed next to the `ricochet` binary.

In portable mode (default), all configuration is stored in a folder called `config` with the binary. When installed, the platform's user configuration path is used instead.

The [buildscripts](https://github.com/ricochet-im/buildscripts) repository contains a set of scripts to build a fully static Ricochet on a clean Debian system. These are used to create the generic linux binary packages.

## OS X

You will need:
 * Xcode (for toolchain)
 * Qt 5 - preferably the [Qt SDK](https://qt-project.org/downloads)

You can either load `ricochet.pro` in Qt Creator and build normally, or build command-line with:
```sh
/path/to/qtsdk/5.3/clang_64/bin/qmake
make
```

You also need a `tor` binary in $PATH or inside the build's `ricochet.app/Contents/MacOS` folder. The easiest solution is to use `brew install tor`. If you copy the `tor` binary, you will need to keep it up to date.

Normally, configuration will be stored in a `config.ricochet` folder, in the same location as `ricochet.app`. However, if the bundle is installed to `/Applications`, the system location `~/Library/Application Support/Ricochet` is used instead. You can force that behavior by adding `DEFINES+=RICOCHET_NO_PORTABLE` to the qmake command.

The `packaging/osx/release_osx.sh` script demonstrates how to build a redistributable app bundle.

## Windows

Building for Windows is difficult. You will need:
 * Visual Studio C++ or MinGW
 * Qt 5 - preferably the [Qt SDK](https://qt-project.org/downloads)
 * OpenSSL (including libs and headers)

After installing the Qt SDK, open the `ricochet.pro` project in Qt Creator. Before building, you must click the 'Projects' tab on the left side, and under 'Build Steps', modify 'Additional arguments' to add: `OPENSSLDIR=C:\path\to\openssl\`. Use the 'Build -> Run qmake' menu to test your changes.

You also need a `tor.exe` binary, placed in the same folder as `ricochet.exe`.

The windows installer can be built using Inno Setup. See `packaging\installer` for more information.
