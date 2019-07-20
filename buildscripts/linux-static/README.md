This is a very simple set of scripts to build Ricochet with a statically linked Qt, Tor, and OpenSSL. It produces a tarball that is generally usable on most modern linux installations.

This is only tested on Debian 7. It builds an entire copy of Qt, which requires several GB of disk space and considerable time.

In your preferred build directory, run `setup.sh` to clone repositories, and `build.sh` to pull, configure, and build them all. If all goes well, src/ricochet/build/ will contain a tar.bz2 at the end.

Contributions to improve this process are welcome.
