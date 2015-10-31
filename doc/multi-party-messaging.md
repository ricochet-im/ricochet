# Multi Party Messaging

Author: Sarah Jamie Lewis
Created: 2015-10-30
Status: Draft


## Overview 

There has been some demand for multi-party (or group) messaging, where a single
client can send and receive responses from multiple users in the context of 
a single chat. This document outlines a design for the Ricochet protocol to
implement multi-party messaging (MPM).


## Design

Alice is a Ricochet user who wishes to initiate a group chat with her contacts,
Bob and Carol.

1. Alice sets up a new hidden service HS and publishes an `openchannel` request to
Bob and Alice with a multi-party context which contains the ricochet id for
HS. 

2. Once Bob and Carol receive the open channel message from Alice they attempt to
make a connection to HS (along with a contact request which is automatically
accepted by HS).

3. Alice also connects to HS.

4. Once connected, HS opens a channel to Alice, Bob and Carol.

At this point the MPM context is complete.

All messages sent to HS are relayed to all the other channels e.g. if Alice 
sends a message, this message is relayed through HS to Bob and Carol.

When the group chat context is no longer required, HS is torn down, and the 
credentials are destroyed.


## New Protocol Elements

For MPM to work a new extension is needed MultiPartyContext:

        package Protocol.Data.MultiPartyContext
        import "ControlChannel.proto";
        
        extend Control.OpenChannel {
            required string id; // ricochet address of the MPM Service
        }


## New Client Elements

The introduction of a new flow of data, requires new UX elements to avoid 
confusion.


### Initiate Group Chat

In most IM client, this functionality is usually found in two places. On the 
contact pane, and in each individual chat window.

I suggest that the Ricochet client follow these guidelines. As such we would
have two new buttons on the UI.

1. In the messaging pane for a conversation we would have a "invite" button 
which would allow the user to select a contact to add to the conversation
 
    a. If the conversation was already an MPM context, the invitee would simply 
    receive the open channel MPM request to the defined hidden service. 

    b. If no such context has been setup the the inviter sets up the MPM context
    and invites both the contact from the original context and the new invitee 
    to the new MPM context. 

2. In the contact pane, selecting multiple contacts and clicking a new 
"Group Chat" button would initiate an MPM context as described in *1b*.


## Changes to the Threat Model

The above design adds some additional threats to the ricochet threat model. 


### Arbitrary Contact Routing 

It is now possible for an authenticated client to cause another client to 
connect to an arbitrary ricochet address.

There are a few ways to mitigate this threat.

1. Introduce a new Authentication Mechanism that requires the new hidden service
to authenticate with both it's private key and the originators key. This requires
that the contact has some element of control over the newly created hidden
service.

2. Force a contact request box for the new hidden service - from a UX perspective
this isn't the nicest - but permission is always sought out.


### Leaking Contact Information

Once nice property of the MPM context outlined above, is that while Alice needs
to know Bob and Carol, Bob and Carol don't need to know, or trust, each other.

This would nicely if the group chat is restricted to a single parties contact
list. However, if it was desirable to include 3rd parties e.g. Bob wants to 
invite their friend Eve to the chat, then Eve has to reveal their ricochet
address to Alice (via the newly setup hidden service).

In this case, it would seem like a permission box would be the right way to 
deal with the scenario. "You are being invited to a group chat hosted by someone
not on your contact list. By connecting you will be revealing your hidden service
address to them. Do you want to continue?"


