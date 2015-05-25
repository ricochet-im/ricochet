# Technical design of Ricochet

Ricochet is an instant messaging system designed around Tor hidden services. This document describes the goals and design of that system from a technical perspective.

The reader should be familiar with [Tor](https://www.torproject.org/about/overview.html.en) and [hidden services](https://www.torproject.org/docs/hidden-services.html.en).

## Goals for the project

To implement a real-time messaging system with these properties:

 * Users aren't personally identifiable by contacts or their address
 * Communication is authenticated and private
 * No person or server can access contact lists, message history, or other metadata
 * Resist censorship and monitoring at the local network level
 * Resist blacklisting or denial of service against users
 * Accessible and understandable for non-technical users
 * Reliability and interactivity comparable with traditional IM services

## Introduction

Each user identity is represented by a hidden service as its connection point. The identity is shared as a contact ID in the form `ricochet:qjj5g7bxwcvs3d7i`, which is unique and sufficient to connect to the service.

When online, the user publishes a hidden service corresponding with the onion hostname in the contact ID, and accepts bidirectionally anonymous connections, which are authenticated as known contacts or used to receive contact requests.

Known contacts use a [customized protocol](https://github.com/ricochet-im/ricochet/blob/master/doc/protocol.md) over these connections for basic instant messaging features.

## Contact requests

Like classic instant messaging systems, you can request to add a user to your contacts using their contact ID. That request must be accepted before messages can be sent or received.

A request is made by connecting to the service, indicating that the connection is for a contact request, and providing information including the sender's contact ID. The sender attempts periodically to make a connection for the contact request when online.

The request includes:

 * The hidden service hostname of the recipient hidden service
 * A random cookie provided at the start of the connection by the recipient
 * A random secret the recipient can use to authenticate normal connections
 * The full public key corresponding to the hidden service for the sender's identity
 * (Optionally) A nickname and short introductory message
 * An RSA signature of the above with the same public key

The recipient can calculate the sender's contact ID based on the public key, and authenticate it by verifying the signature on the request. This proves that the sender can publish the hidden service represented by their contact ID.

The recipient user has the choice to accept or reject that request. A rejected public key may be added to a blacklist and rejected automatically from future requests.

For more detail, see the [protocol documentation](https://github.com/ricochet-im/ricochet/blob/master/doc/protocol.md#contact-request-channel).

## Contact connections

When online, ricochet periodically attempts to make connections to all contacts. If the connection attempt succeeds, it is kept open and that contact is considered online. Only one connection is needed per contact, and it does not matter which end initiated the connection.

The hidden service layer conveniently provides confidentiality, ephemerality, and authenticates the server side, so the application protocol is kept very simple. The client side of a connection authenticates with a pre-shared random secret established during or soon after the contact request.

A simple command/reply based binary protocol is used to communicate. It attempts to offer some reliability for commands to recover from unstable connections. This was chosen over any existing protocol (such as XMPP) or implementation for simplicity and strict control over the surface exposed for attacks against security and anonymity.

The protocol includes a version negotiation step for future expansion. A detailed description of the format and commands can be found in the [protocol documentation](https://github.com/ricochet-im/ricochet/blob/master/doc/protocol.md#introduction-and-version-negotiation).

## UI and usability

User interface is an extremely important and often under-considered aspect of security and anonymity. Less technical users should be able to easily learn how to use the software, and what they need to do to keep themselves safe.

Ricochet's UI aims to be simple and familiar to users of other IM software. Knowledge of Tor and networking concepts aren't a requirement. It should be easy to do things the right way, hard to accidentally break them, and possible for technical users to tweak.

Contributions in this area are very welcome, especially in translations and non-technical documentation and review.

## Future development

The design described here is close to the simplest implementation possible. The protocol has the potential to be extended to enable features like file transfer or even voice/video streaming. More advanced use of hidden services (e.g. authentication) can mitigate the risks of publishing a publicly connectable service. Separate services or more elaborate designs can be used to prevent attacks by non-contacts. Future development in Tor can improve the cryptography and principles behind hidden services. Your ideas can go here.

Ideas, suggestions, and bugs are welcome on the [issue tracker](https://github.com/ricochet-im/ricochet/issues). Patches are welcome via pull request.
