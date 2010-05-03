#ifndef CRYPTOKEY_H
#define CRYPTOKEY_H

#include <QString>

class CryptoKey
{
public:
    CryptoKey();

    bool loadFromFile(const QString &path);
};

#endif // CRYPTOKEY_H
