Torsion IM - http://torsionim.org/ & http://github.com/special/torsion/

Introduction
============

Torsion is an anonymous instant messaging system built around Tor.
It works just like any instant messenger, but nobody (not even your contacts)
can find out who you are, and nobody else can see your conversations.

Torsion is BETA software. Do not expect perfect security and anonymity.
When using Torsion, take appropriate precautions against the possibility of
exploitation or weaknesses that may contribute to attacks against your anonymity.

Please report any bugs or problems to:
    special@dereferenced.net
    irc.oftc.net #torsion (or special)
    qjj5g7bxwcvs3d7i@Torsion

About Tor
=========

Torsion uses Tor's hidden services to connect anonymously with your contacts. For
more information on Tor, visit https://www.torproject.org/. Torsion is produced
independently from the Tor anonymity software. No guarantee is made by the
developers of Torsion or by The Tor Project about the quality, suitability, or
any other aspect of this software.

To support the Tor project and internet anonymity, please consider running a relay.
See https://www.torproject.org/docs/tor-doc-relay.html for more information.

Building
========

Requirements:
    Qt 4.6 or newer (http://qt.nokia.com/)
        Qt 4.5 (such as Ubuntu Karmic) works, but has reduced functionality.
    OpenSSL (http://www.openssl.org/)

First, run:
    qmake Torsion.pro
    -or-
    qmake Torsion.pro OPENSSLDIR=/path/to/openssl

Then, build with your standard build tool; make (Linux) or nmake (Windows),
or the Visual Studio project files. If desired, install it system-wide with
'sudo make install'.

Usage
=====

Torsion can use an existing installation of Tor or the Vidalia Bundle. For best
results, you should install Vidalia prior to running Torsion. The wizard will make
required changes to the Vidalia and Tor configuration to allow Torsion (i.e. changing
the control port configuration).

Some distributions of Torsion may include a bundled copy of Tor. This is recommended
if you run multiple copies of Torsion on the same system, or have existing hidden services.
You can add special configuration options to the Tor/torrc file under Torsion's
installation directory.

After Tor is configured, a contact ID will be generated and published as a hidden service.
The status indicator on your identity should become green within several minutes, but you
can start adding contacts immediately.

You can share your contact ID (something like w3rf2xcq1b88lbda@Torsion) to allow other
people to add you; you will be given the choice of accepting or rejecting any requests.
Once you've added a contact, they will connect automatically whenever they're online.

Technical Information
=====================

Technical documentation, plans, TODOs, and so forth will be available soon. I'm committed
to making Torsion an open, transparent, and inclusive project. Until then, if you have any
questions, feel free to contact me with the information above.