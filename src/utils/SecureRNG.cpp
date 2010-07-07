/* Torsion - http://github.com/special/torsion
 * Copyright (C) 2010, John Brooks <special@dereferenced.net>
 *
 * Torsion is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Torsion. If not, see http://www.gnu.org/licenses/
 */

#include "SecureRNG.h"
#include <QtDebug>
#include <openssl/rand.h>
#include <openssl/err.h>

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
        return QByteArray();

    return re;
}

unsigned SecureRNG::randomInt(unsigned max)
{
    unsigned cutoff = UINT_MAX - (UINT_MAX % max);
    unsigned value = 0;

    for (;;)
    {
        random(reinterpret_cast<char*>(value), sizeof(value));
        if (value < cutoff)
            return value % max;
    }
}

#ifndef UINT64_MAX
#define UINT64_MAX ((quint64)-1)
#endif

quint64 SecureRNG::randomInt64(quint64 max)
{
    quint64 cutoff = UINT64_MAX - (UINT64_MAX % max);
    quint64 value = 0;

    for (;;)
    {
        random(reinterpret_cast<char*>(value), sizeof(value));
        if (value < cutoff)
            return value % max;
    }
}
