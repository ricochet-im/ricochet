#ifndef CRYPTOKEY_H
#define CRYPTOKEY_H

#include <QString>

class CryptoKey
{
public:
    CryptoKey();
    ~CryptoKey();

    static void test(const QString &file);

    bool loadFromFile(const QString &path);
    void clear();

    bool isLoaded() const { return key != 0; }
    QByteArray encodedPublicKey();

private:
    typedef struct rsa_st RSA;

    RSA *key;
};

#endif // CRYPTOKEY_H
