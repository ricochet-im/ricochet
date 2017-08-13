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

#include "CryptoKey.h"
#include "SecureRNG.h"
#include "Useful.h"
#include <QtDebug>
#include <QFile>
#include <openssl/bn.h>
#include <openssl/bio.h>
#include <openssl/pem.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L || defined(LIBRESSL_VERSION_NUMBER)
void RSA_get0_factors(const RSA *r, const BIGNUM **p, const BIGNUM **q)
{
  *p = r->p;
  *q = r->q;
}
#define RSA_bits(o) (BN_num_bits((o)->n))
#endif

void base32_encode(char *dest, unsigned destlen, const char *src, unsigned srclen);
bool base32_decode(char *dest, unsigned destlen, const char *src, unsigned srclen);

CryptoKey::CryptoKey()
{
}

CryptoKey::~CryptoKey()
{
    clear();
}

CryptoKey::Data::~Data()
{
    if (key)
    {
        RSA_free(key);
        key = 0;
    }
}

void CryptoKey::clear()
{
    d = 0;
}

bool CryptoKey::loadFromData(const QByteArray &data, KeyType type, KeyFormat format)
{
    RSA *key = NULL;
    clear();

    if (data.isEmpty())
        return false;

    if (format == PEM) {
        BIO *b = BIO_new_mem_buf((void*)data.constData(), -1);

        if (type == PrivateKey)
            key = PEM_read_bio_RSAPrivateKey(b, NULL, NULL, NULL);
        else
            key = PEM_read_bio_RSAPublicKey(b, NULL, NULL, NULL);

        BIO_free(b);
    } else if (format == DER) {
        const uchar *dp = reinterpret_cast<const uchar*>(data.constData());

        if (type == PrivateKey)
            key = d2i_RSAPrivateKey(NULL, &dp, data.size());
        else
            key = d2i_RSAPublicKey(NULL, &dp, data.size());
    } else {
        Q_UNREACHABLE();
    }

    if (!key) {
        qWarning() << "Failed to parse" << (type == PrivateKey ? "private" : "public") << "key from data";
        return false;
    }

    d = new Data(key);
    return true;
}

bool CryptoKey::loadFromFile(const QString &path, KeyType type, KeyFormat format)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open" << (type == PrivateKey ? "private" : "public") << "key from"
                   << path << "-" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    return loadFromData(data, type, format);
}

bool CryptoKey::isPrivate() const
{
    if (!isLoaded()) {
      return false;
    } else {
        const BIGNUM *p, *q;
        RSA_get0_factors(d->key, &p, &q);
        return (p != 0);
    }
}

int CryptoKey::bits() const
{
    return isLoaded() ? RSA_bits(d->key) : 0;
}

QByteArray CryptoKey::publicKeyDigest() const
{
    if (!isLoaded())
        return QByteArray();

    QByteArray buf = encodedPublicKey(DER);

    QByteArray re(20, 0);
    bool ok = SHA1(reinterpret_cast<const unsigned char*>(buf.constData()), buf.size(),
         reinterpret_cast<unsigned char*>(re.data())) != NULL;

    if (!ok)
    {
        qWarning() << "Failed to hash public key data for digest";
        return QByteArray();
    }

    return re;
}

QByteArray CryptoKey::encodedPublicKey(KeyFormat format) const
{
    if (!isLoaded())
        return QByteArray();

    if (format == PEM) {
        BIO *b = BIO_new(BIO_s_mem());

        if (!PEM_write_bio_RSAPublicKey(b, d->key)) {
            BUG() << "Failed to encode public key in PEM format";
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
    } else if (format == DER) {
        uchar *buf = NULL;
        int len = i2d_RSAPublicKey(d->key, &buf);
        if (len <= 0 || !buf) {
            BUG() << "Failed to encode public key in DER format";
            return QByteArray();
        }

        QByteArray re((const char*)buf, len);
        OPENSSL_free(buf);
        return re;
    } else {
        Q_UNREACHABLE();
    }

    return QByteArray();
}

QByteArray CryptoKey::encodedPrivateKey(KeyFormat format) const
{
    if (!isLoaded() || !isPrivate())
        return QByteArray();

    if (format == PEM) {
        BIO *b = BIO_new(BIO_s_mem());

        if (!PEM_write_bio_RSAPrivateKey(b, d->key, NULL, NULL, 0, NULL, NULL)) {
            BUG() << "Failed to encode private key in PEM format";
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
    } else if (format == DER) {
        uchar *buf = NULL;
        int len = i2d_RSAPrivateKey(d->key, &buf);
        if (len <= 0 || !buf) {
            BUG() << "Failed to encode private key in DER format";
            return QByteArray();
        }

        QByteArray re((const char*)buf, len);
        OPENSSL_free(buf);
        return re;
    } else {
        Q_UNREACHABLE();
    }

    return QByteArray();
}

QString CryptoKey::torServiceID() const
{
    if (!isLoaded())
        return QString();

    QByteArray digest = publicKeyDigest();
    if (digest.isNull())
        return QString();

    static const int hostnameDigestSize = 10;
    static const int hostnameEncodedSize = 16;

    QByteArray re(hostnameEncodedSize+1, 0);
    base32_encode(re.data(), re.size(), digest.constData(), hostnameDigestSize);

    // Chop extra null byte
    re.chop(1);

    return QString::fromLatin1(re);
}

QByteArray CryptoKey::signData(const QByteArray &data) const
{
    QByteArray digest(32, 0);
    bool ok = SHA256(reinterpret_cast<const unsigned char*>(data.constData()), data.size(),
                   reinterpret_cast<unsigned char*>(digest.data())) != NULL;
    if (!ok) {
        qWarning() << "Digest for RSA signature failed";
        return QByteArray();
    }

    return signSHA256(digest);
}

QByteArray CryptoKey::signSHA256(const QByteArray &digest) const
{
    if (!isPrivate())
        return QByteArray();

    QByteArray re(RSA_size(d->key), 0);
    unsigned sigsize = 0;
    int r = RSA_sign(NID_sha256, reinterpret_cast<const unsigned char*>(digest.constData()), digest.size(),
                     reinterpret_cast<unsigned char*>(re.data()), &sigsize, d->key);

    if (r != 1) {
        qWarning() << "RSA encryption failed when generating signature";
        return QByteArray();
    }

    re.truncate(sigsize);
    return re;
}

bool CryptoKey::verifyData(const QByteArray &data, QByteArray signature) const
{
    QByteArray digest(32, 0);
    bool ok = SHA256(reinterpret_cast<const unsigned char*>(data.constData()), data.size(),
                     reinterpret_cast<unsigned char*>(digest.data())) != NULL;

    if (!ok) {
        qWarning() << "Digest for RSA verify failed";
        return false;
    }

    return verifySHA256(digest, signature);
}

bool CryptoKey::verifySHA256(const QByteArray &digest, QByteArray signature) const
{
    if (!isLoaded())
        return false;

    int r = RSA_verify(NID_sha256, reinterpret_cast<const uchar*>(digest.constData()), digest.size(),
                       reinterpret_cast<uchar*>(signature.data()), signature.size(), d->key);
    if (r != 1)
        return false;
    return true;
}

/* Cryptographic hash of a password as expected by Tor's HashedControlPassword */
QByteArray torControlHashedPassword(const QByteArray &password)
{
    QByteArray salt = SecureRNG::random(8);
    if (salt.isNull())
        return QByteArray();

    int count = ((quint32)16 + (96 & 15)) << ((96 >> 4) + 6);

    SHA_CTX hash;
    SHA1_Init(&hash);

    QByteArray tmp = salt + password;
    while (count)
    {
        int c = qMin(count, tmp.size());
        SHA1_Update(&hash, reinterpret_cast<const void*>(tmp.constData()), c);
        count -= c;
    }

    unsigned char md[20];
    SHA1_Final(md, &hash);

    /* 60 is the hex-encoded value of 96, which is a constant used by Tor's algorithm. */
    return QByteArray("16:") + salt.toHex().toUpper() + QByteArray("60") +
           QByteArray::fromRawData(reinterpret_cast<const char*>(md), 20).toHex().toUpper();
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

     /* We need an even multiple of 5 bits, and enough space */
    if ((nbits%5) != 0 || destlen > (nbits/5)+1) {
        Q_ASSERT(false);
        memset(dest, 0, destlen);
        return;
    }

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

     /* We need an even multiple of 8 bits, and enough space */
    if ((nbits%8) != 0 || (nbits/8)+1 > destlen) {
        Q_ASSERT(false);
        return false;
    }

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
