### Anonymous and serverless instant messaging that just works
Torsion is an experiment with a different kind of instant messaging that **doesn't trust anyone** with your identity, your contact list, or your communications.

* You can chat without exposing your identity (or IP address) to *anyone*
* Nobody can discover who your contacts are or when you talk (*metadata-free!*)
* There are no servers to compromise or operators to intimidate for your information
* It's cross-platform and easy for non-technical users

### How it works
Torsion is a peer-to-peer instant messaging system based on Tor [hidden services](https://www.torproject.org/docs/hidden-services.html.en). Your login is your hidden service address, and contacts connect to you (not an intermediate server) through Tor. The rendezvous system makes it extremely hard for anyone to learn your identity from your address.

Torsion is not affiliated with or endorsed by The Tor Project.

For more information, you can [read about Tor](https://www.torproject.org/about/overview.html.en) and [learn about Torsion's design](???) or [protocol](https://github.com/special/torsion/blob/master/doc/protocol.txt). Everything is [open-source](https://github.com/special/torsion/blob/master/LICENSE) and open to contribution.

### Experimental
This software is an experiment. It hasn't been audited or formally reviewed by anyone. Security and anonymity are difficult topics, and you should carefully evaluate your risks and exposure with any software. *Do not rely on Torsion for your safety* unless you have more trust in my work than it deserves. That said, I believe it does more to try to protect your privacy than any similar software.

### Downloading & Building
#### Building from source
To build for Linux, OS X, or Windows, you need:
 * Qt 5.1 or later
 * OpenSSL
 * A pre-built Tor binary and its dependencies

Place `tor` in your build directory or PATH and run `qmake` (or `qmake-qt5`). `make`, and `make install` if desired. To build packages, see the scripts under the `packaging` directory.

### Other
Bugs can be reported on the [issue tracker](https://github.com/special/torsion/issues).

You can contact me directly at `john.brooks@dereferenced.net` (PGP [183C045D](http://pgp.mit.edu/pks/lookup?op=get&search=0xFF97C53F183C045D)).

You should support [The Tor Project](https://www.torproject.org/donate/donate.html.en), [The Internet Defense League](https://www.internetdefenseleague.org/), [EFF](https://www.eff.org/), and [run a Tor relay](https://www.torproject.org/docs/tor-relay-debian.html.en).
