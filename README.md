### Anonymous and serverless instant messaging that just works
Ricochet is an experiment with a different kind of instant messaging that **doesn't trust anyone** with your identity, your contact list, or your communications.

* You can chat without exposing your identity (or IP address) to *anyone*
* Nobody can discover who your contacts are or when you talk (*metadata-free!*)
* There are no servers to compromise or operators to intimidate for your information
* It's cross-platform and easy for non-technical users

### How it works
Ricochet is a peer-to-peer instant messaging system built on Tor [hidden services](https://www.torproject.org/docs/hidden-services.html.en). Your login is your hidden service address, and contacts connect to you (not an intermediate server) through Tor. The rendezvous system makes it extremely hard for anyone to learn your identity from your address.

Ricochet is not affiliated with or endorsed by The Tor Project.

For more information, you can [read about Tor](https://www.torproject.org/about/overview.html.en) and [learn about Ricochet's design](https://github.com/ricochet-im/ricochet/blob/master/doc/design.md) or [protocol](https://github.com/ricochet-im/ricochet/blob/master/doc/protocol.txt). Everything is [open-source](https://github.com/ricochet-im/ricochet/blob/master/LICENSE) and open to contribution.

### Experimental
This software is an experiment. It hasn't been audited or formally reviewed by anyone. Security and anonymity are difficult topics, and you should carefully evaluate your risks and exposure with any software. *Do not rely on Ricochet for your safety* unless you have more trust in my work than it deserves. That said, I believe it does more to try to protect your privacy than any similar software.

### Downloading & Building
#### Build requirements
 * Qt 5.1 or later (see Linux notes below)
 * OpenSSL
 * A pre-built Tor binary and its dependencies

Place `tor` or `tor.exe` in your build directory or PATH. To build packages, see the scripts under the `packaging` directory.

#### Linux
Users of Ubuntu 14.04 or earlier and other slow distributions will need to use the [Qt SDK](https://qt-project.org/downloads) or build their own Qt.

Run `qmake` or `qmake-qt5`, then `make`. The default build portable, which will store configuration in a folder named `config` next to the binary. For a system installation using XDG configuration directories, run `qmake DEFINES+=RICOCHET_NO_PORTABLE` instead.

#### OS X
Use the [Qt SDK](https://qt-project.org/downloads) or homebrew. Run `qmake` and `make` to build an application bundle. The default build will store configuration in a `config.ricochet` folder next to the application *unless* the path looks like a system-wide Applications folder, in which case `~/Library/Application Support/Ricochet` is used.

#### Windows
Builds with MinGW or MSVC. You will need the [Qt SDK](https://qt-project.org/downloads) and a copy of OpenSSL headers and libraries.

You must pass `OPENSSLDIR="C:\Path\To\OpenSSL\Build"` to qmake. If using Qt Creator, add it to Additional arguments in the Projects/Build Settings tab. The default build is portable and stores configuration in a `config` folder next to the binary. Pass `DEFINES+=RICOCHET_NO_PORTABLE` to qmake to use the user appdata location instead.

### Other
Bugs can be reported on the [issue tracker](https://github.com/ricochet-im/ricochet/issues). Translations can be contributed on [Transifex](https://www.transifex.com/projects/p/torsion/).

You can contact me with `ricochet:rs7ce36jsj24ogfw` or `john.brooks@dereferenced.net` (PGP [183C045D](http://pgp.mit.edu/pks/lookup?op=get&search=0xFF97C53F183C045D)).

You should support [The Tor Project](https://www.torproject.org/donate/donate.html.en), [The Internet Defense League](https://www.internetdefenseleague.org/), [EFF](https://www.eff.org/), and [run a Tor relay](https://www.torproject.org/docs/tor-relay-debian.html.en).
