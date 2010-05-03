#include "CryptoKey.h"
#include "SecureRNG.h"
#include <QtDebug>
#include <QFile>
#include <openssl/bio.h>
#include <openssl/pem.h>

void base32_encode(char *dest, unsigned destlen, const char *src, unsigned srclen);
bool base32_decode(char *dest, unsigned destlen, const char *src, unsigned srclen);

CryptoKey::CryptoKey()
    : key(0)
{
}

CryptoKey::~CryptoKey()
{
    clear();
}

void CryptoKey::clear()
{
    if (key)
    {
        RSA_free(key);
        key = 0;
    }
}

bool CryptoKey::loadFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open private key from" << path << "-" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    BIO *b = BIO_new_mem_buf((void*)data.constData(), -1);
    key = PEM_read_bio_RSAPrivateKey(b, NULL, NULL, NULL);

    BIO_free(b);

    if (!key)
    {
        qDebug() << "Failed to parse private key from" << path;
        return false;
    }

    return true;
}

bool CryptoKey::isPrivate() const
{
    return key && key->p != 0;
}

QByteArray CryptoKey::publicKeyDigest() const
{
    if (!isLoaded())
        return QByteArray();

    int len = i2d_RSAPublicKey(key, NULL);
    if (len < 0)
        return QByteArray();

    QByteArray buf;
    buf.resize(len);
    unsigned char *bufp = reinterpret_cast<unsigned char*>(buf.data());

    len = i2d_RSAPublicKey(key, &bufp);
    if (len < 0)
    {
        qDebug() << "Failed to encode public key for digest";
        return QByteArray();
    }

    QByteArray re;
    re.resize(20);

    bool ok = SHA1(reinterpret_cast<const unsigned char*>(buf.constData()), buf.size(),
         reinterpret_cast<unsigned char*>(re.data())) != NULL;

    if (!ok)
    {
        qDebug() << "Failed to hash public key data for digest";
        return QByteArray();
    }

    return re;
}

QByteArray CryptoKey::encodedPublicKey() const
{
    if (!isLoaded())
        return QByteArray();

    BIO *b = BIO_new(BIO_s_mem());

    if (!PEM_write_bio_RSAPublicKey(b, key))
    {
        qDebug() << "Failed to encode public key";
        BIO_free(b);
        return QByteArray();
    }

    BUF_MEM *buf;
    BIO_get_mem_ptr(b, &buf);

    /* Close BIO, but don't free buf. */
    (void)BIO_set_close(b, BIO_NOCLOSE);
    BIO_free(b);

    QByteArray re((const char *)buf->data, (int)buf->length);
    BUF_MEM_free(buf);

    return re;
}

QString CryptoKey::torServiceID() const
{
    if (!isLoaded())
        return QString();

    QByteArray digest = publicKeyDigest();
    if (digest.isNull())
        return QString();

    QByteArray re;
    re.resize(17);

    base32_encode(re.data(), 17, digest.constData(), 10);

    return QString::fromLatin1(re);
}

QByteArray CryptoKey::signData(const QByteArray &data) const
{
    if (!isPrivate())
        return QByteArray();

    QByteArray re;
    re.resize(RSA_size(key));

    int r = RSA_private_encrypt(data.size(), reinterpret_cast<const unsigned char*>(data.constData()),
                                reinterpret_cast<unsigned char*>(re.data()), key, RSA_PKCS1_PADDING);

    if (r < 0)
    {
        qDebug() << "RSA encryption failed when generating signature";
        return QByteArray();
    }

    re.resize(r);
    return re;
}

bool CryptoKey::verifySignature(const QByteArray &data, const QByteArray &signature) const
{
    if (!isLoaded())
        return false;

    QByteArray re;
    re.resize(RSA_size(key) - 11);

    int r = RSA_public_decrypt(signature.size(), reinterpret_cast<const unsigned char*>(signature.constData()),
                               reinterpret_cast<unsigned char*>(re.data()), key, RSA_PKCS1_PADDING);

    if (r < 0)
    {
        qDebug() << "RSA decryption failed when verifying signature";
        return false;
    }

    re.resize(r);

    return (re == data);
}

void CryptoKey::test(const QString &file)
{
    CryptoKey key;

    bool ok = key.loadFromFile(file);
    Q_ASSERT(ok);
    Q_ASSERT(key.isLoaded());

    QByteArray pubkey = key.encodedPublicKey();
    Q_ASSERT(!pubkey.isEmpty());

    qDebug() << "(crypto test) Encoded public key:" << pubkey;

    QByteArray pubdigest = key.publicKeyDigest();
    Q_ASSERT(!pubdigest.isEmpty());

    qDebug() << "(crypto test) Public key digest:" << pubdigest.toHex();

    QString serviceid = key.torServiceID();
    Q_ASSERT(!serviceid.isEmpty());

    qDebug() << "(crypto test) Tor service ID:" << serviceid;

    QByteArray data = SecureRNG::random(16);
    Q_ASSERT(!data.isNull());
    qDebug() << "(crypto test) Random data:" << data.toHex();

    QByteArray signature = key.signData(data);
    Q_ASSERT(!signature.isNull());
    qDebug() << "(crypto test) Signature:" << signature.toHex();

    ok = key.verifySignature(data, signature);
    qDebug() << "(crypto test) Verified signature:" << ok;
}

/* Copyright (c) 2001-2004, Roger Dingledine
 * Copyright (c) 2004-2006, Roger Dingledine, Nick Mathewson
 * Copyright (c) 2007-2010, The Tor Project, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 *
 *   Neither the names of the copyright owners nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
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

#define BASE32_CHARS "abcdefghijklmnopqrstuvwxyz234567"

/* Implements base32 encoding as in rfc3548. Requires that srclen*8 is a multiple of 5. */
void base32_encode(char *dest, unsigned destlen, const char *src, unsigned srclen)
{
    unsigned i, bit, v, u;
    unsigned nbits = srclen * 8;

    Q_ASSERT((nbits%5) == 0); /* We need an even multiple of 5 bits. */
    Q_ASSERT((nbits/5)+1 <= destlen); /* We need enough space. */

    for (i = 0, bit = 0; bit < nbits; ++i, bit += 5)
    {
        /* set v to the 16-bit value starting at src[bits/8], 0-padded. */
        v = ((quint8) src[bit / 8]) << 8;
        if (bit + 5 < nbits)
            v += (quint8) src[(bit/8)+1];

        /* set u to the 5-bit value at the bit'th bit of src. */
        u = (v >> (11 - (bit % 8))) & 0x1F;
        dest[i] = BASE32_CHARS[u];
    }

    dest[i] = '\0';
}

/* Implements base32 decoding as in rfc3548. Requires that srclen*5 is a multiple of 8. */
bool base32_decode(char *dest, unsigned destlen, const char *src, unsigned srclen)
{
    unsigned int i, j, bit;
    unsigned nbits = srclen * 5;

    Q_ASSERT((nbits%8) == 0); /* We need an even multiple of 8 bits. */
    Q_ASSERT((nbits/8) <= destlen); /* We need enough space. */

    char *tmp = new char[srclen];

    /* Convert base32 encoded chars to the 5-bit values that they represent. */
    for (j = 0; j < srclen; ++j)
    {
        if (src[j] > 0x60 && src[j] < 0x7B)
            tmp[j] = src[j] - 0x61;
        else if (src[j] > 0x31 && src[j] < 0x38)
            tmp[j] = src[j] - 0x18;
        else if (src[j] > 0x40 && src[j] < 0x5B)
            tmp[j] = src[j] - 0x41;
        else
        {
            delete[] tmp;
            return false;
        }
    }

    /* Assemble result byte-wise by applying five possible cases. */
    for (i = 0, bit = 0; bit < nbits; ++i, bit += 8)
    {
        switch (bit % 40)
        {
        case 0:
            dest[i] = (((quint8)tmp[(bit/5)]) << 3) + (((quint8)tmp[(bit/5)+1]) >> 2);
            break;
        case 8:
            dest[i] = (((quint8)tmp[(bit/5)]) << 6) + (((quint8)tmp[(bit/5)+1]) << 1)
                      + (((quint8)tmp[(bit/5)+2]) >> 4);
            break;
        case 16:
            dest[i] = (((quint8)tmp[(bit/5)]) << 4) + (((quint8)tmp[(bit/5)+1]) >> 1);
            break;
        case 24:
            dest[i] = (((quint8)tmp[(bit/5)]) << 7) + (((quint8)tmp[(bit/5)+1]) << 2)
                      + (((quint8)tmp[(bit/5)+2]) >> 3);
            break;
        case 32:
            dest[i] = (((quint8)tmp[(bit/5)]) << 5) + ((quint8)tmp[(bit/5)+1]);
            break;
        }
    }

    delete[] tmp;
    return true;
}
