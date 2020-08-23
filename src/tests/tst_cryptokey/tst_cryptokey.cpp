/* Ricochet - https://ricochet.im/
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

// C
#include <stdio.h>
#include <string.h>
// C++
#include <QtTest>

// libtego
#include <tego/tego.hpp>

// libtego_ui
#include <utils/CryptoKey.h>
#include <utils/SecureRNG.h>

class TestCryptoKey : public QObject
{
    Q_OBJECT

private slots:
    void loadFromServiceId();
    void loadFromKeyBlob();
    void encodedKeyBlob();
    void torServiceId();
    void signData();
    void verifYData();
};

constexpr char keyBlob[] = "ED25519-V3:CAeUhUcyrjvk95WmTaexNRY5+0wFvd7P2zDMhhBZM2TwnD2I9YgK3yMO/jOk0LVc39xnULCR02ZBghiyFdNR3w==";

constexpr char serviceId[] = "ockeilzymnguehc4brf4dpcsc634wtei75wa5edslx6yuwfaw3pje6id";

constexpr char message[] = "Hello, How are you?";

constexpr uint8_t signature[] = {
    0xf2, 0x88, 0x9a, 0x40, 0x35, 0xa6, 0xc5, 0xca,
    0x7c, 0x11, 0xcf, 0xd3, 0x3a, 0x5e, 0x75, 0xea,
    0xac, 0x53, 0x6f, 0x31, 0xf0, 0x78, 0x8a, 0x8f,
    0x0e, 0x55, 0x86, 0x60, 0x18, 0x96, 0x3a, 0x8a,
    0x44, 0x15, 0xef, 0xe6, 0x03, 0x67, 0x6d, 0xb2,
    0x6f, 0x90, 0x81, 0x22, 0x76, 0x77, 0xa2, 0xcb,
    0x39, 0xcc, 0xef, 0x67, 0xb7, 0x33, 0x73, 0x21,
    0xc0, 0x10, 0xe9, 0x37, 0x14, 0x93, 0x37, 0x05,
};

void TestCryptoKey::loadFromServiceId()
{
    CryptoKey ck;
    QVERIFY(!ck.isLoaded());

    // give loadFromServiceId bad data

    // nullptr
    QVERIFY_EXCEPTION_THROWN(ck.loadFromServiceId(nullptr), std::runtime_error);
    QVERIFY(!ck.isLoaded());

    // empty string
    QVERIFY_EXCEPTION_THROWN(ck.loadFromServiceId(""), std::runtime_error);
    QVERIFY(!ck.isLoaded());

    // a random string
    QVERIFY_EXCEPTION_THROWN(ck.loadFromServiceId("garbage string"), std::runtime_error);
    QVERIFY(!ck.isLoaded());

    // correct length but invalid service id
    QVERIFY_EXCEPTION_THROWN(ck.loadFromServiceId("fakeserviceidfakeserviceidfakeserviceidfakeserviceidfake"), std::runtime_error);
    QVERIFY(!ck.isLoaded());

    // load a valid service id
    QVERIFY(ck.loadFromServiceId(serviceId));
    QVERIFY(ck.isLoaded());
    QVERIFY(!ck.isPrivate());
}

void TestCryptoKey::loadFromKeyBlob()
{
    CryptoKey ck;
    QVERIFY(!ck.isLoaded());

    // giving loadFromKeyBlob bad data

    // nullptr
    QVERIFY_EXCEPTION_THROWN(ck.loadFromKeyBlob(nullptr), std::runtime_error);
    QVERIFY(!ck.isLoaded());

    // empty string
    QVERIFY_EXCEPTION_THROWN(ck.loadFromKeyBlob(""), std::runtime_error);
    QVERIFY(!ck.isLoaded());

    // a random string
    QVERIFY_EXCEPTION_THROWN(ck.loadFromKeyBlob("garbage string"), std::runtime_error);
    QVERIFY(!ck.isLoaded());

    // correct prefix not enough data
    QVERIFY_EXCEPTION_THROWN(ck.loadFromKeyBlob("ED25519-V3:looksLikeBase64"), std::
        runtime_error);
    QVERIFY(!ck.isLoaded());
    // right size, bad prefix
    QVERIFY_EXCEPTION_THROWN(ck.loadFromKeyBlob("BAD-PREFIX:/RU9USuBKBRmPRkLBXbN/cVbDUoz7+Ay/aCIX4jTyQ9GM5UBaMwDvfzt21DEv5NhUxt61GpZf06z6iN0WElwgA=="), std::runtime_error);
    QVERIFY(!ck.isLoaded());

    // load a valid KeyBlob
    QVERIFY(ck.loadFromKeyBlob(keyBlob));
    QVERIFY(ck.isLoaded());
    QVERIFY(ck.isPrivate());
}

void TestCryptoKey::encodedKeyBlob()
{
    CryptoKey ck;
    QVERIFY(!ck.isLoaded());

    // try to encode when no key loaded
    QVERIFY_EXCEPTION_THROWN(ck.encodedKeyBlob(), std::runtime_error);

    // load private key from KeyBlob
    QVERIFY(ck.loadFromKeyBlob(keyBlob));
    QVERIFY(ck.isLoaded());

    // verify it round trips
    QCOMPARE(ck.encodedKeyBlob(), keyBlob);
}

void TestCryptoKey::torServiceId()
{
    CryptoKey ck;
    QVERIFY(!ck.isLoaded());

    // try to get service id when no key loaded
    QVERIFY_EXCEPTION_THROWN(ck.torServiceID(), std::runtime_error);

    // load private key from KeyBlob
    QVERIFY(ck.loadFromKeyBlob(keyBlob));
    QCOMPARE(ck.isLoaded(), true);
    QCOMPARE(ck.isPrivate(), true);

    // compare calculated service id to truth
    QCOMPARE(ck.torServiceID(), serviceId);

    // clear out key
    ck.clear();
    QVERIFY(!ck.isLoaded());

    // load public key from service id
    QVERIFY(ck.loadFromServiceId(serviceId));
    QCOMPARE(ck.isLoaded(), true);
    QVERIFY(!ck.isPrivate());

    // compare calculated service id to truth
    QCOMPARE(ck.torServiceID(), serviceId);
}

void TestCryptoKey::signData()
{
    CryptoKey ck;
    QVERIFY(!ck.isLoaded());

    // fail to sign with no key
    QVERIFY_EXCEPTION_THROWN(ck.signData(message), std::runtime_error);

    // fail to sign with a public key
    QVERIFY(ck.loadFromServiceId(serviceId));
    QVERIFY(ck.isLoaded());
    QVERIFY(!ck.isPrivate());
    QVERIFY_EXCEPTION_THROWN(ck.signData(message), std::runtime_error);

    ck.clear();
    QVERIFY(!ck.isLoaded());

    // load our private key
    QVERIFY(ck.loadFromKeyBlob(keyBlob));
    QVERIFY(ck.isLoaded());
    QVERIFY(ck.isPrivate());

    // fail to sign empty message
    QVERIFY_EXCEPTION_THROWN(ck.signData({}), std::runtime_error);

    // succed to sign our message
    QCOMPARE(ck.signData(message), QByteArray(reinterpret_cast<const char*>(signature), sizeof(signature)));
}
void TestCryptoKey::verifYData()
{
    CryptoKey ck;
    QVERIFY(!ck.isLoaded());

    // load public key from service id
    QVERIFY(ck.loadFromServiceId(serviceId));
    QVERIFY(ck.isLoaded());
    QVERIFY(!ck.isPrivate());

    // try and verify an empty signature
    QVERIFY_EXCEPTION_THROWN(ck.verifyData(message, {}), std::runtime_error);
    // try and verify signature with not enough bytes (32 rather than 64)
    QVERIFY_EXCEPTION_THROWN(ck.verifyData(message, QByteArray{32, 0}), std::runtime_error);
    // try and verify incorrect sig (64 0s)
    QVERIFY(!ck.verifyData(message, QByteArray{64, 0}));

    // verify correct signature
    QVERIFY(ck.verifyData(message, QByteArray(reinterpret_cast<const char*>(signature), sizeof(signature))));
}

QTEST_MAIN(TestCryptoKey)
#include "tst_cryptokey.moc"
