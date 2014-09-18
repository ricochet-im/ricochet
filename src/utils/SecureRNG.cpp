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

#include "SecureRNG.h"
#include <QtDebug>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <limits.h>

#ifdef Q_OS_WIN
#include <Wincrypt.h>
#endif

#if QT_VERSION >= 0x040700
#include <QElapsedTimer>
#endif

bool SecureRNG::seed()
{
#if QT_VERSION >= 0x040700
    QElapsedTimer timer;
    timer.start();
#endif

#ifdef Q_OS_WIN
    /* RAND_poll is very unreliable on windows; with older versions of OpenSSL,
     * it can take up to several minutes to run and has been known to crash.
     * Even newer versions seem to take around 400ms, which is far too long for
     * interactive startup. Random data from the windows CSP is used as a seed
     * instead, as it should be very high quality random and fast. */
    HCRYPTPROV provider = 0;
    if (!CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
    {
        qWarning() << "Failed to acquire CSP context for RNG seed:" << hex << GetLastError();
        return false;
    }

    /* Same amount of entropy OpenSSL uses, apparently. */
    char buf[32];

    if (!CryptGenRandom(provider, sizeof(buf), reinterpret_cast<BYTE*>(buf)))
    {
        qWarning() << "Failed to get entropy from CSP for RNG seed: " << hex << GetLastError();
        CryptReleaseContext(provider, 0);
        return false;
    }

    CryptReleaseContext(provider, 0);

    RAND_seed(buf, sizeof(buf));
    memset(buf, 0, sizeof(buf));
#else
    if (!RAND_poll())
    {
        qWarning() << "OpenSSL RNG seed failed:" << ERR_get_error();
        return false;
    }
#endif

#if QT_VERSION >= 0x040700
    qDebug() << "RNG seed took" << timer.elapsed() << "ms";
#endif

    return true;
}

bool SecureRNG::random(char *buf, int size)
{
    int r = RAND_bytes(reinterpret_cast<unsigned char*>(buf), size);
    if (!r)
    {
        // FIXME: This should be fatal
        qWarning() << "RNG failed:" << ERR_get_error();
        return false;
    }

    return true;
}

QByteArray SecureRNG::random(int size)
{
    QByteArray re;
    re.resize(size);

    if (!random(re.data(), size))
        // FIXME: This is really unsafe!
        return QByteArray();

    return re;
}

QByteArray SecureRNG::randomPrintable(int length)
{
    QByteArray re = random(length);
    for (int i = 0; i < re.size(); i++)
        re[i] = randomInt(95) + 32;
    return re;
}

unsigned SecureRNG::randomInt(unsigned max)
{
    unsigned mask = 0;
    unsigned value = 0;

    for (int i = max; i > 0; i >>= 1) {
        mask++;
    }
    if (mask == 32) {
        mask = 0xffffffff;
    } else {
        mask = (1<<mask) - 1;
    }
    
    for (;;)
    {
        if(random(reinterpret_cast<char*>(&value), sizeof(value))) {
            value &= mask;
            if (value <= max) {
                return value;
            }
        }
    }
}

#ifndef UINT64_MAX
#define UINT64_MAX ((quint64)-1)
#endif

quint64 SecureRNG::randomInt64(quint64 max)
{
    quint64 mask = 0;
    quint64 value = 0;

    for (int i = max; i > 0; i >>= 1) {
        mask++;
    }
    if (mask == 64) {
        mask = 0xffffffffffffffff;
    } else {
        mask = ((quint64)1<<mask) - 1;
    }
    
    for (;;)
    {
        if(random(reinterpret_cast<char*>(&value), sizeof(value))) {
            value &= mask;
            if (value <= max) {
                return value;
            }
        }
    }
}
