http://torsionim.org/

Torsion is an anonymous instant messaging system built around Tor.
It works just like any instant messenger, but nobody (not even your contacts)
can find out who you are, and nobody else can see your conversations.

Torsion is beta software; please report any bugs or problems to 
john.brooks@dereferenced.net, special @ irc.oftc.net, or qjj5g7bxwcvs3d7i@Torsion

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

Torsion uses an existing installation of Tor or the Vidalia Bundle. For best results,
you should install Vidalia prior to running Torsion. The wizard will make required
changes to the Vidalia and Tor configuration to allow Torsion. 

After Tor is successfully configured, a contact ID will be generated and published
as a hidden service. The identity status should become green within several minutes,
indicating that it's fully connected.

You can share your contact ID (something like w3rf2xcq1b88lbda@Torsion) to allow other
people to add you; you will be given the choice of accepting or rejecting any requests.
Once you've added a contact, they will connect automatically whenever they're online.

Licensing & Modifications
=========================

Torsion is released under the GNU General Public License, version 2 or later. You are
free to modify and redistribute the software under the terms of this license. An additional
exception is applied to allow redistribution with the OpenSSL toolkit. See the LICENSE and
LICENSE-GPLv2 files for details.

Patches are accepted and encouraged, and can be submitted through GitHub or via email.

Technical Information
=====================

Technical documentation, plans, TODOs, and so forth will be available soon. I'm committed
to making Torsion an open, transparent, and inclusive project. Until then, if you have any
questions, feel free to contact me with the information above.