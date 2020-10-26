# WORK IN PROGRESS PROJECT REPOSITORY - NOT FOR PRODUCTION USE

## Tego-Core
Tego-Core is the work in progress repository for Ricochet Refresh. We are in the process of making significant changes to Ricochet Refresh to implement V3 Onion Service support as well as some exciting new features. The change to V3 Onion Services will not be backwards compatible with V2, which is the version currently implemented in Ricochet Refresh. As such, we are keeping development separate until we are ready to release a V3 Onion Services version of Ricochet Refresh. 

As noted above, this is a work in progress repository and should not be used in production. Security protocols may not be fully implemented or tested during the development process. 

Existing README is continued below.

## R<sup>2</sup>: Ricochet Refresh
Ricochet Refresh is the new updated version of Ricochet, supported by Blueprint for Free Speech.. We are a non-government, not-for-profit organisation working to safeguard the freedom of expression for whistleblowers, activists, and everybody else, worldwide. To find out more, check out our profile, or head to [http://blueprintforfreespeech.net]. Blueprint was the original sponsor of Ricochet, written by developer J. Brooks.

Ricochet Refresh is currently available for OS X (10.12 or later) and Linux, with a Windows release available real soon now (tm). Visit the [releases page](https://github.com/blueprint-freespeech/ricochet-refresh/releases) for the latest version and changelog.

Details about Ricochet can be found at [the original repository](https://github.com/ricochet-im/ricochet). The readme for Ricochet is reproduced below for convenience.

## Ricochet's README (not by or affiliated with Blueprint)
### Anonymous metadata-resistant instant messaging that just works.
Ricochet is an experimental kind of instant messaging that **doesn't trust anyone** with your identity, your contact list, or your communications.

* You can chat without exposing your identity (or IP address) to *anyone*
* Nobody can discover who your contacts are or when you talk (*metadata-free!*)
* There are no servers or operators that could be compromised, exposing your information.
* It's cross-platform and easy for non-technical users.

![Screenshot](ricochetscreen.png)

### How it works
Ricochet is a peer-to-peer instant messaging system built on the Tor Network [hidden services](https://www.torproject.org/docs/hidden-services.html.en). Your login is your hidden service address, and contacts connect to you (not an intermediate server) through Tor. The rendezvous system makes it extremely hard for anyone to learn your identity from your address.

Ricochet is not affiliated with or endorsed by The Tor Project.

For more information, you can [read about Tor](https://www.torproject.org/about/overview.html.en) and [learn about Ricochet's design](https://github.com/ricochet-im/ricochet/blob/master/doc/design.md) or [protocol](https://github.com/ricochet-im/ricochet/blob/master/doc/protocol.md) (or the [old protocol](https://github.com/ricochet-im/ricochet/blob/master/doc/deprecated/protocol-1.0.txt)). Everything is [open-source](https://github.com/ricochet-im/ricochet/blob/master/LICENSE) and open to contribution.

### Experimental
This software is an experiment. Security and anonymity are difficult topics, and you should carefully evaluate your risks and exposure with any software. *Do not rely on Ricochet for your safety* unless you have more trust in my work than it deserves. That said, I believe it does more to try to protect your privacy than any similar software, and is the best chance you have of withholding your personal information.

### Downloads

Ricochet is available for Windows, OS X (10.7 or later), and as a generic Linux binary package. Visit the [releases page](https://github.com/ricochet-im/ricochet/releases) for the latest version and changelog.

All releases and signatures are also available at https://ricochet.im/releases/.

### Building from source
See [BUILDING](https://github.com/blueprint-freespeech/ricochet-refresh/blob/master/BUILDING.md) for Linux, OS X, and Windows build instructions.

### Other
Bugs can be reported on the [issue tracker](https://github.com/blueprint-freespeech/ricochet-refresh/issues). Translations can be contributed on [Transifex](https://www.transifex.com/projects/p/ricochet/).

You should support [The Tor Project](https://www.torproject.org/donate/donate.html.en), [EFF](https://www.eff.org/), and [run a Tor relay](https://www.torproject.org/docs/tor-relay-debian.html.en).
