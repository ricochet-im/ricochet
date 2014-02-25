/* Torsion - http://torsionim.org/
 * Copyright (C) 2014, John Brooks <john.brooks@dereferenced.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 *    * Neither the names of the copyright owners nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef PROTOCOLCONSTANTS_H
#define PROTOCOLCONSTANTS_H

namespace Protocol {

enum {
    ProtocolVersion = 0,
    MaxCommandSize = 65535,
    HeaderSize = 6,
    MaxCommandData = 65535 - HeaderSize
};

// Connection purpose
enum Purpose {
    PurposePrimary    = 0x00,
    PurposeContactReq = 0x80
};

// Contact requests
enum {
    RequestCookieSize = 16
};

enum RequestResponse {
    RequestAcknowledged = 0x00,
    RequestAccepted     = 0x01,
    RequestRejected     = 0x40,
    RequestSyntaxError  = 0x80,
    RequestVerifyError  = 0x81,
    RequestEmptyError   = 0x82,
    RequestVersionError = 0x90
};

// Command state
enum StateFlags {
    Reply   = 0x80,
    Final   = 0x40,
    Success = 0xa0
};

enum State {
    // Standardized final errors
    GenericError           = 0xd0,
    UnknownCommand         = 0xd1,
    UnknownState           = 0xd2,
    MessageSyntaxError     = 0xd3,
    CommandSyntaxError     = 0xd4,
    InternalError          = 0xd5,
    PermanentInternalError = 0xd6,
    ConnectionError        = 0xd7
};

// Convert value to a command state
inline quint8 commandState(quint8 value)
{
    Q_ASSERT(!(value & 0xc0));
    return (value & (~0x80)) | 0x40;
}

// Convert value to a reply state
inline quint8 replyState(bool success, bool final, quint8 value)
{
    Q_ASSERT(!(value & 0xe0));
    if (!success)
        Q_ASSERT(!(value & 0xf0));

    value |= 0x80;

    if (final)
        value |= 0x40;
    else
        value &= ~0x40;

    if (success)
        value |= 0x20;
    else
        value &= ~0x20;

    return value;
}

inline bool isReply(quint8 state)
{
    return (state & 0x80) == 0x80;
}

inline bool isSuccess(quint8 state)
{
    return (state & 0xa0) == 0xa0;
}

inline bool isFinal(quint8 state)
{
    return (state & 0x40) == 0x40;
}

}

#endif

