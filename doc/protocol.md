## Overview

Ricochet is a peer-to-peer instant messaging system built on anonymity networks. This document
defines the communication protocol between two Ricochet instances, as carried out over a Tor hidden
service connection.

The protocol is defined in three layers:

The **connection layer** describes the use of an anonymized TCP-style connection for peer-to-peer
communication.

The **packet layer** separates the connection into a series of *packets* delivered to *channels*.
This allows multiplexing different operations on the same connection, and packetizes data for
channel-level parsing.

The **channel layer** parses and handles packets according to the *channel type* and the state of
that specific channel.

### Connections

> TODO: This is a brief explanation; we should reference a design/architecture document with more
> details.

##### Hidden services

Ricochet uses Tor [hidden services][rend-spec] as a transport; the reader should be familiar with
that architecture and the properties it provides. In particular:

 * The hostname is calculated from a hash of the server's public key, and serves to authenticate the
   server without relying on a third party
 * Connections are encrypted end-to-end, using the server's key and a DHE handshake to provide
   forward secrecy
 * Both ends of a connection are anonymous in that neither peer should be able to identify or locate
   the other, and no relay should be able to connect an identity to the requests it makes
 * Impersonating a server without its private key requires an 80-bit SHA1 collision using a valid
   RSA key

> TODO: We should explore additional cryptography on top of what Tor offers; see
> [issue 72](https://github.com/ricochet-im/ricochet/issues/72).

##### Usage

Each Ricochet instance publishes a hidden service, which serves as its identity and accepts
connections from contacts. When it first comes online, it attempts to connect to the addresses of
known contacts. If a connection is made, it is held open; a contact is considered online when there
is an open connection. Connections are made on port 9878.

> This solution isn't ideal; we'll be exploring better designs on top of hidden services to improve
> scalability and anonymity properties.

Only one active connection is needed for a contact. Connections are fully bidirectional and all
behavior is equivalent regardless of which peer acts as server at the transport level.

Ricochet does not use central servers; connections are made to services published directly by your
contacts with no intermediary.

Keeping open connections to unknown peers poses a risk for various attacks, including resource
exhaustion. Clients must either authenticate or take other useful action (e.g. delivering a contact
request) quickly. The server side of the connection should expire unknown connections.

### Channels

Channels divide up the connection to allow multiplexing, extensibility, and stateful behavior for
*packets*.

The **channel id** associates packets with an instance of a channel on the connection, which was
previously created by an *OpenChannel* message.

The **channel type** defines how packets are parsed and handled. Distinct features have separate
channel types; for example, `im.ricochet.chat` and `im.ricochet.file-transfer`. By convention,
these are in reverse-URI form.

Channels exist within a connection. The channel ID is unique only within that connection, and all
channels are closed when the connection is lost.

Channels must be explicitly created with an *OpenChannel* message. The recipient of that message
chooses to accept or reject the channel; for example, it may reject channel types it doesn't
support, or won't allow this peer to access.

Channel instances also provide a state for messages. For example, all operations associated with the
transfer of one file take place on the same channel, and a second file transfer would use a second
channel of the same type.

Both peers may send packets to the same channel. Depending on the channel type, messages may be
fully bidirectional or may be a command-response protocol.

At the beginning of the connection, one channel exists automatically: the *control channel*. As a
special case, it always has a channel ID of `0`. The control channel provides functionality for
creating new channels and maintenance of the underlying connection.

### Authentication

Ricochet needs a variety of levels and types of authentication; known contacts might have a strong
proof of identity, while a request from a new person comes with a different proof and an anti-spam
"proof of work". Some features could allow unauthenticated use.

To support these scenarios, there is no pre-protocol authentication step. Peers add credentials to
their connection by opening and completing various types of authentication channels.

The most common example is `im.ricochet.auth.hidden-service`: the peer creates a channel of this
type and carries out its protocol to prove that it has the private key for a hidden service name.
Afterwards, that peer can send a contact request, and the recipient is able to know the source of
that request.

Another hypothetical example is `example.hashcash`: the peer would complete a proof of work as
evidence that it doesn't intend to spam the recipient.

These credentials are associated with the connection. For example, you may decide to not allow an
`im.ricochet.chat` channel unless the peer has completed `im.ricochet.auth.hidden-service`
authentication for a known contact's address.

The hidden service transport provides one special case: the server end of the connection is
authenticated equivalent to `im.ricochet.auth.hidden-service` at the beginning of the connection,
and must be given equivalent privileges.

### Conventions

Unless otherwise noted, these conventions and definitions are used for the protocol and this
document:

* *Peer* refers to either Ricochet instance on a connection
* *Recipient* refers to the peer which received the message
* Channels encode data using [protocol buffers][protobuf], with one protobuf message per packet
* Unless the channel type specifies another mechanism, unknown/unparsable messages result in
  closing the channel.
* Protocol behavior which appears malicious or strange may trigger consequences
* Strings are UTF-8 encoded and should be carefully validated and handled
* Any reply may be artificially delayed, but order must be preserved

## Specification

### Introduction and version negotiation

Immediately after establishing a connection, the client side must send an introduction message
identifying versions of the protocol that it is able to use. The server responds with one of
those versions, or an error indicating that no compatible version exists.

This step exists to enable smoother protocol changes in the future, and for better compatibility
with old clients.

The client begins the connection by sending the following raw sequence of bytes:

```
0x49
0x4D
nVersions          // One byte, number of supported protocol versions, must be at least 1
nVersions times:
    version        // One byte, protocol version number
```

The total size is 3 plus the number of supported versions bytes. The number of supported versions must be at least 1. The server side of the connection
must respond with a single byte for the selected version number, or 0xFF if no suitable version
is found.

This document describes protocol version 1. Known versions are:
```
0                  The Ricochet 1.0 protocol
1                  This document
```

If the negotiation is successful, the connection can be immediately used to begin exchanging messages
(the packet layer, below).

### Packet layer

The base layer on the connection is a trivial packet structure:

```
uint16 size        // Big endian, including the header bytes
uint16 channel     // Big endian, channel identifier
bytes  data        // Content of the packet
```

The connection reads and buffers data until it has a full packet, then looks up the channel
identifier within the list of open channels. If the channel is found, data is passed to it for
parsing and handling.

The only other functionality implemented at this layer is closing a channel. A channel is closed by
sending a packet to that channel with 0 bytes of data. When a packet is received for an unknown
channel, the recipient responds by closing that channel.

Note that packets are limited to 65,535 bytes in size, including the 4-byte header. To avoid causing
latency on low throughput connections, channels should keep packets as small as possible. If a
channel type requires larger packets of data, it must define a way to reassemble them specific to
that channel type.

### Control channel

The control channel is a special case: it is the only channel open from the beginning of a
connection, and it is always assigned the channel identifier `0`. If the control channel is closed,
the connection must also terminate.

The control channel contains methods used for maintenance of the connection and the creation of
other channels. It is a stateless series of protobuf-serialized `ControlMessage`, with one message
encoded per packet. Both peers on the connection may send all messages.

##### Packet
```protobuf
message Packet {
    // Must contain exactly one field
    optional OpenChannel open_channel = 1;
    optional ChannelResult channel_result = 2;
    optional KeepAlive keep_alive = 3;
    optional EnableFeatures enable_features = 4;
    optional FeaturesEnabled features_enabled = 5;
}
```

All packets sent to the control channel must encode a *Packet*, with exactly one field.

##### OpenChannel
```protobuf
message OpenChannel {
    required int32 channel_identifier = 1;      // Arbitrary unique identifier for this channel instance
    required string channel_type = 2;           // String identifying channel type; e.g. im.ricochet.chat

    // It is valid to extend the OpenChannel message to add fields specific
    // to the requested channel_type.
    extensions 100 to max;
}
```

Requests to open a channel of the type *channel_type*, using the identifier *channel_identifier* for
packets. Additional data may be added in extensions to this message.

The recipient of an OpenChannel message checks whether it supports the *channel_type*, if the
*channel_identifier* is valid and unassigned, and the validity of any extension data. The recipient
also checks whether this connection allows channels of this type; for example, if the peer is
sufficiently authenticated.

If the request is allowed, *channel_identifier* will be sent with packets destined for this channel
within this connection. It is also used to associate the *ChannelResult* message with this request.
There are several rules that must be followed when choosing or accepting a *channel_identifier*:

* The client side of a connection may only open odd-numbered channels
* The server side may only open even-numbered channels
* The identifier must fit within the range of uint16
* The identifier must not be used by an open channel
* The identifier should increase for every OpenChannel message, wrapping if necessary. Identifiers
  should not be re-used except after wrapping.

The even/odd restrictions on *channel_identifier* prevent peers from racing to open a channel with
the same id. Channels are tied to a specific connection, so there is no collision between connections.

By convention, channel types are in reverse URI format, e.g. `im.ricochet.chat`.

A *ChannelResult* message must always be generated in response. If the request is egregiously
invalid, the connection may be terminated instead.

##### ChannelResult
```protobuf
message ChannelResult {
    required int32 channel_identifier = 1;      // Matching the value from OpenChannel
    required bool opened = 2;                   // If the channel is now open

    enum CommonError {
        GenericError = 0;
        UnknownTypeError = 1;
        UnauthorizedError = 2;
        BadUsageError = 3;
        FailedError = 4;
    }

    optional CommonError common_error = 3;

    // As with OpenChannel, it is valid to extend this message with fields specific
    // to the channel type.
    extensions 100 to max;
}
```

Sent in response to an *OpenChannel* message, with the same *channel_identifier* value. If *opened*
is true, the channel is now ready to accept packets tagged with that identifier.

##### KeepAlive
```protobuf
message KeepAlive {
    required bool response_requested = 1;
}
```

A simple ping message. If *response_requested* is true, a *KeepAlive* message is generated in
response with *response_requested* as false.

##### EnableFeatures
```protobuf
message EnableFeatures {
    repeated string feature = 1;
    extensions 100 to max;
}

message FeaturesEnabled {
    repeated string feature = 1;
    extensions 100 to max;
}
```

Simple feature negotiation. Either peer may send the *EnableFeatures* message with a list of
strings representing protocol changes or features. The recipient must respond with *FeaturesEnabled*
containing the subset of those strings it recognizes and has enabled.

No such feature strings are currently defined, and the current implementation should always respond
with an empty list.

### Chat channel

| Channel            | Detail |
| ------------------ | ------ |
| **Channel type**   | `im.ricochet.chat` |
| **Purpose**        | Sending text-based instant messages |
| **Direction**      | One-way: Only initiator of the channel sends commands, and recipient sends replies |
| **Singleton**      | Only one chat channel is created by each peer on the connection |
| **Authentication** | Requires `im.ricochet.auth.hidden-service` as a known contact |

A chat channel allows the initiator (the peer who created the channel) to send messages, and receive
acknowledgement for those messages. The opposing peer should also create a chat channel to send its
own chat messages. Acknowledgement must be on the same channel as the original message. One peer may
not open more than one chat channel on the same connection.

Two chat channels (one per peer) are used to avoid ambiguity on which peer creates a chat channel,
or which channel would be used in a race situation.

##### Packet
```protobuf
message Packet {
    optional ChatMessage chat_message = 1;
    optional ChatAcknowledge chat_acknowledge = 2;
}
```

##### ChatMessage
```protobuf
message ChatMessage {
    required string message_text = 1;
    optional uint32 message_id = 2;                // Random ID for ack
    optional int64 time_delta = 3;                 // Delta in seconds between now and when message was written
}
```

A *message_id* of zero (or omitted) indicates that the recipient doesn't expect acknowledgement.

If *message_id* is non-zero, the recipient should acknowledge receiving this message by sending
*ChatAcknowledge*. Unacknowledged messages may be re-sent with the same *message_id*, and the
recipient should drop duplicate messages with an identical non-zero *message_id*, after sending an
acknowledgement.

Sometimes, messages may be delayed or potentially lost across connections over a short period of time.
In order to allow messages to be re-sent after a lost connection, clients should try to avoid choosing
message ids from a recent connection (with the same peer) at the start of a new connection. For
example, that can be done by randomizing the first message id for a channel.

The *time_delta* field is a delta in seconds between when the message was composed and when it is being
transmitted. For messages that are sent immediately, it should be 0 or omitted. If a message was written
and couldn't be sent immediately (due to a connection failure, for example), the *time_delta* should be
an approximation of when it was composed. A positive value does not make any sense, as it would indicate
a message composed in the future.

##### ChatAcknowledge
```protobuf
message ChatAcknowledge {
    optional uint32 message_id = 1;
    optional bool accepted = 2 [default = true];
}
```

Acknowledge receipt of a *ChatMessage*.

The *accepted* parameter indicates whether or not the message is to be
considered delivered to the client. If it is false, then the message delivery
should be considered to have failed.

### Contact request channel

| Channel            | Detail |
| ------------------ | ------ |
| **Channel type**   | `im.ricochet.contact.request` |
| **Purpose**        | Introduce a new client and ask for user approval to send messages |
| **Direction**      | One-way: Only initiator of the channel sends commands, and recipient sends replies |
| **Singleton**      | One instance created only by the client side of a connection |
| **Authentication** | Requires `im.ricochet.auth.hidden-service` |

Contact requests are sent to introduce oneself to the recipient and ask for further contact,
including being put on the recipient's persistent contact list.

The requesting client must have authenticated using `im.ricochet.auth.hidden-service` to prove
ownership of a hidden service name.

The recipient isn't required to immediately respond to a request. If the request is approved, the
recipient may connect to the requesting client, and that is treated as implicitly accepting the
request.

##### OpenChannel
```protobuf
extend Control.OpenChannel {
    optional ContactRequest contact_request = 200;
}

extend Control.ChannelResult {
    optional Response response = 201;
}
```

The OpenChannel message on a contact request channel must include the `contact_request` extension. A
successful ChannelResult must include the `response` extension.

If the response finishes the request, the channel will be closed immediately. Otherwise, the channel
remains open to wait for another *Response* message (e.g. going from Pending to Accepted).

##### ContactRequest
```protobuf
// Sent only as an attachment to OpenChannel
message ContactRequest {
    optional string nickname = 1;
    optional string message_text = 2;
}
```

Deliver a contact request, usually with a message and nickname attached. The "identity" of the
request is proven through `im.ricochet.auth.hidden-service` authentication.

The request is sent as an extension on the *OpenChannel* message.

##### Response
```protobuf
message Response {
    enum Status {
        Undefined = 0; // Not valid in transmitted messages
        Pending = 1;
        Accepted = 2;
        Rejected = 3;
        Error = 4;
    }

    required Status status = 1;
}
```

Indicates the status of a contact request. The *Pending* status may be followed by another
*ContactRequestResponse* with a final status. All other statuses must be followed by closing the
channel, and may also close the connection. Closing the channel or the connection does not imply
having a response - for example, the recipient may decide to time out the connection while it is
waiting in the *Pending* state.

The initial *Response* is sent as an extension to the *ChannelResult* message when the channel is
opened. If that response is final, the channel is closed immediately after. Otherwise, the channel
remains open, and the only valid message is another *Response*.

If a request is *Rejected*, the requesting client must not send that request again, unless the user
has manually cancelled the previous request and made a new one. Recipients should automatically
reject excessive or abusive requests.

If an *Error* occurs, the requesting client may only request again if it believes the error is
solved. Recipients should automatically reject requests after repetitive errors.

### AuthHiddenService

| Channel            | Detail |
| ------------------ | ------ |
| **Channel type**   | `im.ricochet.auth.hidden-service` |
| **Purpose**        | Authenticate as the owner of a Tor hidden service |
| **Direction**      | One-way: Only initiator of the channel sends commands, and recipient sends replies |
| **Singleton**      | One instance created only by the client side of a connection |
| **Authentication** | No prior authentication required |

The `im.ricochet.auth.hidden-service` channel is used to prove ownership of a hidden service name by
demonstrating ownership of a matching private key. This is used to authenticate as a known contact,
or to prove ownership of a service name before sending a contact request.

As a result of the transport, the server side of a hidden service connection is considered to have
automatically completed `im.ricochet.auth.hidden-service` authentication, and must be allowed
equivalent access.

##### Packet
```protobuf
extend OpenChannel {
    optional bytes client_cookie = 7200;      // 16 random bytes
}

extend ChannelResult {
    optional bytes server_cookie = 7200;      // 16 random bytes
}

message Packet {
    optional Proof proof = 1;
    optional Result result = 2;
}
```

The channel is opened by the peer who wishes to authenticate itself. The *OpenChannel* message
must contain a *client_cookie* of 16 bytes. A successful *ChannelResult* message must include
the *server_cookie* field, with a randomly generated value used to prevent replayed authentication.

##### Proof
```protobuf
message Proof {
    optional bytes public_key = 1;      // DER encoded RSA public key
    optional bytes signature = 2;       // RSA signature
}
```

The proof is calculated as:

```
// + represents concatenation, and function is HMAC-SHA256(key, message)
HMAC-SHA256(client_cookie + server_cookie,
    client_hostname       // base32-encoded client address, without .onion
    + recipient_hostname  // base32-encoded server address, without .onion
)
```

This proof is signed with the hidden service's private key using PKCS #1 v2.0 (as per OpenSSL
RSA_sign) to make *signature*.

The recipient of this message must:

* Reject any message with a public_key field too large or not correctly formed to be a DER-encoded
  1024-bit RSA public key
* Reject any message with a signature field of an unexpected size
* Decode the public_key, and calculate its 'onion' address per [rend-spec][rend-spec]
* Build the proof message
* Verify that *signature* is a valid signature of the proof by *public_key*

##### Result
```protobuf
message Result {
    required bool accepted = 1;
    optional bool is_known_contact = 2;
}
```

If authentication is successful as a known contact, whose connection will be allowed to remain open
without any further purpose, the *is_known_contact* flag must be set as true. If this flag is not
set, the authenticating client should assume that it is not authorized (except e.g. to send a
contact request).

After sending *Result*, the channel should be closed.

[rend-spec]: https://gitweb.torproject.org/torspec.git/blob/HEAD:/rend-spec.txt
[protobuf]: https://code.google.com/p/protobuf/
