#ifndef SECURERNG_H
#define SECURERNG_H

#include <QByteArray>

class SecureRNG
{
public:
    static bool seed();

    static bool random(char *buf, int size);
    static QByteArray random(int size);

    static unsigned randomInt(unsigned max);
    static quint64 randomInt64(quint64 max);
};

#endif // SECURERNG_H
