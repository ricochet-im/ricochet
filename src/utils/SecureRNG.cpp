#include "SecureRNG.h"
#include <QtDebug>
#include <openssl/rand.h>
#include <openssl/err.h>

#if QT_VERSION >= 0x040700
#include <QElapsedTimer>
#endif

bool SecureRNG::seed()
{
#if QT_VERSION >= 0x040700
    QElapsedTimer timer;
    timer.start();
    int r = RAND_poll();
    qDebug() << "RNG seed took" << timer.elapsed() << "ms";
#else
    int r = RAND_poll();
#endif

    if (!r)
    {
        qWarning() << "RNG seed failed:" << ERR_get_error();
        return false;
    }

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
