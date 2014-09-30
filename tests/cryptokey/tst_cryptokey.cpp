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

#include <QtTest>
#include "utils/CryptoKey.h"

class TestCryptoKey : public QObject
{
    Q_OBJECT

private slots:
    void load();
    void publicKeyDigest();
    void encodedPublicKey();
    void torServiceID();
    void sign();
};

const char *alice =
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIICXQIBAAKBgQDAS9nLWyK0jWZ8yduqVEhSyZRplTaeUpGWYRi14n1C4sjO6nqm\n"
    "ES31UCGDH4nIor2R/XMJCJkJwK+t2XrtiH+jUEHwUGhnMkm3hW5NHt5g39s9YK7l\n"
    "xD39O8N2tHUycVq8guhrb1WBQ2/bmZ85nOIuBDZxIuVQZA1U1L6rWGvm+wIDAQAB\n"
    "AoGAewYL6JX9thVgpCVga7BQNObSFFpp/xBEJDkqXfLwwIHmhrpsjSIgjPke94yN\n"
    "0daMAYJsvjLJ9ftYaZjhlGXngbBJiAU95gcZoTAsn2hNJP22ndGuhi6WEKhYwRxK\n"
    "U5d+3Khzy/ysuoay7DSVtpSmpiacWPSiiptEkxNbcbGba8ECQQDeEGoPASmxZoh4\n"
    "I2JNQkqSwMKsOZpp/SJhnmLCPoA1oDwlGtu4HF7t9hBXeyIXgLvbfJudFEa+LqR7\n"
    "wrKQPn0fAkEA3a7cR7eSRNu1ak7gVfQfnP4tFl3+7UC2hUqVHLA5ks4pLl7/ITa+\n"
    "3P04SOs3WpvZJHYJ+hi/anqEPYrD/3B+pQJBAKmjnnHh8IjODDjCxyjAGJntWYoZ\n"
    "4yVOtEIgrc830delley+jNUkDzz3+dnqfcu4k0oD8hjYUYaduRe2T5Szt/8CQQDC\n"
    "EVt8WUNujp0R9P1FohKu4IFeLGmJD/b5V2KUm927HEpG8xkM3Z1XX0KP64MpCnid\n"
    "B80SKeog8CKmsb2F+NiVAkBT1CEAdiFYtf72hnZCLBw5HrqpN+zjw00GjtlrmmNV\n"
    "+ILb/YRp5flCY5Se95ExzQqRKzvK5iJg0yEOVF0OcbO+\n"
    "-----END RSA PRIVATE KEY-----";
const char *aliceDigest = "623a1ffc94d8f8edcd5e47fbd45e08deb911d1bc";
const char *aliceTorID = "mi5b77eu3d4o3tk6";

const char *bob =
    "-----BEGIN RSA PUBLIC KEY-----\n"
    "MIGJAoGBAMP8GyAg/kzwXizpUWjWIMw/lvDffXjsxcq1qmZWZxXJQH/oE8bX+WAf\n"
    "VS8iUHVqTykubR0W3QNL6aWSZKBqDQUTN0QBJUF4qdkg3x56C0kwcWa+seDMAvJw\n"
    "pcHK9wN7mtWHIhFwhikP//NylrY1MaUxcPjvOKcdJ90k988nnmpZAgMBAAE=\n"
    "-----END RSA PUBLIC KEY-----\n";
const char *bobDigest = "b4780cabdfc3593004431644977cf73bf8475848";
const char *bobTorID = "wr4azk67ynmtabcd";

void TestCryptoKey::load()
{
    CryptoKey key;
    QVERIFY(!key.isLoaded());

    // Private key
    QVERIFY(key.loadFromData(alice, CryptoKey::PrivateKey));
    QVERIFY(key.isLoaded());
    QVERIFY(key.isPrivate());
    key.clear();
    QVERIFY(!key.isLoaded());

    // Public key
    QVERIFY(key.loadFromData(bob, CryptoKey::PublicKey));
    QVERIFY(key.isLoaded());
    QVERIFY(!key.isPrivate());
    key.clear();

    // Invalid key
    QVERIFY(!key.loadFromData(QByteArray(alice).mid(0, 150), CryptoKey::PrivateKey));
    QVERIFY(!key.isLoaded());
}

void TestCryptoKey::publicKeyDigest()
{
    CryptoKey key;
    QVERIFY(key.loadFromData(bob, CryptoKey::PublicKey));
    QCOMPARE(key.publicKeyDigest().toHex(), QByteArray(bobDigest));

    key.clear();
    QVERIFY(key.loadFromData(alice, CryptoKey::PrivateKey));
    QCOMPARE(key.publicKeyDigest().toHex(), QByteArray(aliceDigest));
}

void TestCryptoKey::encodedPublicKey()
{
    CryptoKey key;
    QVERIFY(key.loadFromData(bob, CryptoKey::PublicKey));

    QByteArray encoded = key.encodedPublicKey();
    QVERIFY(encoded.contains("PUBLIC KEY"));

    CryptoKey key2;
    QVERIFY(key2.loadFromData(encoded, CryptoKey::PublicKey));
    QCOMPARE(key.encodedPublicKey(), key2.encodedPublicKey());
    QCOMPARE(key.publicKeyDigest(), key2.publicKeyDigest());

    // Doesn't contain a private key
    CryptoKey key3;
    QVERIFY(!key3.loadFromData(encoded, CryptoKey::PrivateKey));
}

void TestCryptoKey::torServiceID()
{
    CryptoKey key;
    QVERIFY(key.loadFromData(bob, CryptoKey::PublicKey));

    QString id = key.torServiceID();
    QCOMPARE(id.size(), 16);
    QCOMPARE(id, QLatin1String(bobTorID));
}

void TestCryptoKey::sign()
{
    CryptoKey key;
    QVERIFY(key.loadFromData(alice, CryptoKey::PrivateKey));

    QByteArray data = QCryptographicHash::hash("test data", QCryptographicHash::Sha1);
    QByteArray data2 = QCryptographicHash::hash("different", QCryptographicHash::Sha1);
    QCOMPARE(data.size(), 20);

    // Good signature
    QByteArray signature = key.signData(data);
    QVERIFY(!signature.isEmpty());
    QVERIFY(key.verifySignature(data, signature));

    // Bad signature
    QVERIFY(!key.verifySignature(data2, signature));

    // Corrupt signature
    QVERIFY(!key.verifySignature(data, signature.mid(0, signature.size() - 10)));

    // Wrong public key
    CryptoKey key2;
    QVERIFY(key2.loadFromData(bob, CryptoKey::PublicKey));
    QVERIFY(!key2.verifySignature(data, signature));
}

QTEST_MAIN(TestCryptoKey)
#include "tst_cryptokey.moc"
